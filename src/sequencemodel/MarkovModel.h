/***************************************************************************
 *   Copyright (C) 2011   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __MARKOVMODEL_H__
#define __MARKOVMODEL_H__

#include <limits>
#include <map>

#include "xasmlib/IntegerVector.h"

#include "sequencemodel/SequenceModel.h"

#include <vector>

#include "json/json.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>

namespace sequencemodel {

#ifndef MARKOV_RNG_BUFFERSIZE
#define MARKOV_RNG_BUFFERSIZE 1024
#endif

/**
 * Simple Markov model class: count kmer frequencies in a set of sequences
 *
 * template parameter bpc: bits per character (default: 2)
 *
 * _bpc must be smaller than 64/32, depending on the system architecture. More practically,
 * the limit is probably memory for storing all possible kmers.
 * 
 * We store things explicitly here for simplicity, using a hash might be a good idea for 
 * higher orders. Memory complexity is:
 * 
 *     2^(_order * _bpc + 1) * sizeof (int)
 *
 */
template <int _bpc = 2, int _out_bpc = 8>
class MarkovModel : public SequenceModel < 
	utilities::IntegerVector<_out_bpc> 
> {
public:

	typedef utilities::IntegerVector<_bpc> internal_string;
	typedef utilities::IntegerVector<_out_bpc> output_string;

	enum {
		CHARACTERS = 1 << _bpc,
	};
	
	/**
	 * Default constructor. Initialize with zero counts.
	 */
	void defaults () {
		BPC = _bpc;
		ORDER = 1;
		KMERS = 1 << ( 1 * _bpc );
		set_seed(1234);
		frequencies.resize(KMERS*CHARACTERS);
		cumulative_probs.resize(KMERS*CHARACTERS);
		rng_pos = MARKOV_RNG_BUFFERSIZE;
		current_kmer = (UINT64) ( random() * KMERS );
		has_cumulative_probs = false;
	}

	virtual ~MarkovModel () {
	}

	/**
	 * overloaded learn method
	 */
	void learn (std::istream & str) {
		internal_string current(ORDER);
		size_t l = 0;
		// DNA map ACGT -> 0123
		int map [26] = {
//			a   b   c   d   e   f   g   h   i   j   k
			0, -1,  1, -1, -1, -1,  2, -1, -1, -1, -1,
//          l   m   n   o   p   q   r   s   t   u   v
		   -1, -1, -1, -1, -1, -1,  1, -1,  3, -1, -1,
//          w   x   y   z
			-1, -1, -1, -1,
		};

		int ch;
		bool newline = true;
		while ( str.good() ) {
			ch = tolower (str.get()) - 'a';
			if (!str.good()) {
				return;
			}
			// skip whitespace
			if (ch == ' ' || ch == '\t') {
				continue;
			}
			if (ch == '\n' || ch == '\r') {
				newline = true;
				continue;
			}
			// ignore fasta comment lines
			if ( newline && ( ch == ';' || ch == '>' ) ) {
				std::string temp;
				std::getline(str, temp, "\n");
				continue;
			}

			// not a legal character -> restart
			if (ch < 0 || ch >= 26) {
				l = 0;
				current.zero();
				continue;
			}
			ch = map[ch];
			if (ch < 0) {
				l = 0;
				current.zero();
				continue;
			}
			
			UINT64 c = ch;
			if (l++ >= ORDER) {
				observe_transition(get_kmer_index(current), c);
			}
			current >>= _bpc; 
			current[ORDER-1] = c;
		}
	}

	/**
	 * Learn model by adding frequencies observed in a sequence
	 */
	void learn (output_string const & sequence) {
		internal_string current(ORDER);

		for (size_t l = 0; l < sequence.size(); ++l) {
			UINT64 c = sequence[l];
			ASSERT (c < (1l << _bpc));
			if (l >= ORDER) {
				observe_transition(get_kmer_index(current), c);
			}
			current >>= _bpc; 
			current[ORDER-1] = c;
		}
	}

	/*
	 * Get the number of observed kmers
	 */
	size_t get_kmer_count ( UINT64 kmer ) const {
		size_t sum = 0;
		kmer <<= _bpc;

		for (size_t j = 0; j < CHARACTERS; ++j) {
			sum+= frequencies[kmer + j];
		}
		return sum;
	}

	size_t get_transition_frequency (UINT64 kmer, UINT64 character) const {
		kmer <<= _bpc;
		kmer+= character;
		return frequencies[kmer];
	}

	double get_transition_probability (UINT64 kmer, UINT64 character) const {
		// no observed kmers: assume equal chances of getting each character
		if (get_kmer_count(kmer) == 0) {
			return 1.0 / CHARACTERS;
		}

		return ((double)get_transition_frequency (kmer, character)) / get_kmer_count(kmer);
	}

	/**
	 * Get the index for a kmer string
	 */
	static UINT64 get_kmer_index (internal_string kmer) {
		return ( ( (UINT64 *) kmer.datavector().exact_data(0) )[0] );
	}

	/**
	 * Get the kmer string for an index
	 */
	internal_string get_kmer_string (UINT64 kmer) {
		internal_string s(ORDER);
		((UINT64*)s.datavector().exact_data(0) )[0] = kmer;
		return s;
	}

	/**
	 * add a transition to the model : character was observed following kmer 
	 */
	void observe_transition (UINT64 kmer, UINT64 character, size_t count = 1) {
		// mark cumulative probabilities for recomputation
		has_cumulative_probs = false;
		kmer <<= _bpc;
		kmer|= character;
		frequencies[kmer]+= count;
	}

	/**
	 * Merge models
	 */
	void merge (MarkovModel<_bpc> const & model) {
		ASSERT (ORDER == model.ORDER);
		has_cumulative_probs = false;

		for (size_t j = 0; j < KMERS * CHARACTERS; ++j) {
			frequencies[j] += model.frequencies[j];
		}
	}

	/**
	 * Generate random sequence
	 */
	output_string generate_sequence (size_t length) {
		if (!has_cumulative_probs) {
			make_cumulative_probs();
		}

		// randomly pick first kmer
		output_string str (length);

		for (size_t j = 1; j <= length; ++j) {
			double rv = random();
			UINT64 ch = 0;
			// random transition into next state
			for (ch = 0; ch < CHARACTERS-1; ++ch) {
				if ( rv < cumulative_probs[current_kmer*CHARACTERS + ch] ) {
					break;
				}
			}
			str[j-1] = ch;
			// next state
			current_kmer >>= _bpc;
			ch <<= (ORDER-1) * _bpc;
			current_kmer |= ch;
		}

		return str;
	}

	/************************************************************************/
	/* Getters/setters                                                      */
	/************************************************************************/
	int order() const { return ORDER; }
	void order(int val) { 
		ORDER = val; 
		KMERS = 1 << ( ORDER * _bpc );

		frequencies.resize(KMERS * CHARACTERS);
	}

	UINT32 get_seed() const { return seed; }
	void set_seed(UINT32 val) { 
		seed = val; 
		rng_state.seed(val);
	}

	/************************************************************************/
	/* Helpers                                                              */
	/************************************************************************/
	void dump ( std::ostream & o ){
		using namespace std;
		UINT64 kmer = 0;
		UINT64 character = 0;

		o << "kmer";
		for (character = 0; character < MarkovModel<_bpc>::CHARACTERS; ++character) {
			o << "\t" << std::hex << std::setfill('0') << std::setw((_bpc+3)>>2) << character << "\t";
		}
		o << std::dec << endl;

		for (kmer = 0; kmer < KMERS; ++kmer) {
			o << get_kmer_string(kmer);
			for (character = 0; character < MarkovModel< _bpc>::CHARACTERS; ++character) {
				o   << "\t"
					<< get_transition_probability(kmer, character) << "\t"
					<< get_transition_frequency(kmer, character);
			}
			o << endl;
		}
	}

private:

	void make_cumulative_probs () {
		cumulative_probs.resize(KMERS * CHARACTERS );

		for (UINT64 kmer = 0; kmer < KMERS; ++kmer) {
			double cp = 0;
			for (UINT64 ch = 0; ch < CHARACTERS; ++ch) {
				cp+= get_transition_probability(kmer, ch);
				cumulative_probs[kmer * CHARACTERS + ch] = cp;
			}
		}
		has_cumulative_probs = true;
	}

	/**
	 * Buffered random number generation
	 */
	double random () {
		if (rng_pos >= MARKOV_RNG_BUFFERSIZE) {
			boost::random::uniform_real_distribution< > rng_dist(0.0, 1.0);
			boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >
    			generateRandomNumbers(rng_state, rng_dist);

			for (int i = 0; i < MARKOV_RNG_BUFFERSIZE; ++i)
			{
				rng_buffer[i] = generateRandomNumbers();
			}
			rng_pos = 0;
		}
		return rng_buffer[rng_pos++];
	}

private:
	/************************************************************************/
	/* Data members                                                         */
	/************************************************************************/

	int BPC;	///< verify _bpc on serialization
	int KMERS;	///< number of KMERS
	int ORDER;  ///< order of the model

	//////////////////////////////////////////////////////////////////////////
	// Counts and probabilities
	std::vector < size_t > frequencies;
	std::vector < double > cumulative_probs;
	bool has_cumulative_probs;

	//////////////////////////////////////////////////////////////////////////
	// Random numbers
	UINT32 seed;	///< random seed
	boost::random::mt19937 rng_state;

	double rng_buffer [MARKOV_RNG_BUFFERSIZE];
	size_t rng_pos;
	UINT64 current_kmer;

	/************************************************************************/
	/* Serialization code                                                   */
	/************************************************************************/
	JSONIZE (MarkovModel, 1,
		S_STORE (seed, JSONUInt <UINT32>)
		S_STORE (BPC, JSONUInt <>)	
		S_STORE (ORDER, JSONInt <>)
		S_STORE (rng_pos, JSONInt <size_t>)
		S_STORE (rng_state, JSONBinaryViaStream < boost::random::mt19937 >)
		S_STORE (current_kmer, JSONUInt <UINT64>)
		S_STORE (rng_buffer, JSONStaticArray < JSONDouble <>, MARKOV_RNG_BUFFERSIZE > )
		S_STORE (frequencies, JSONArray < JSONInt <size_t> > )
		);

	void post_serialize () {
		has_cumulative_probs = false;
		if (BPC != _bpc) {
			throw std::runtime_error ("Error when deserializing: String character size mismatch");
		}

		KMERS = 1 << ( ORDER * _bpc );
		if ( KMERS * CHARACTERS != frequencies.size() ) {
			throw std::runtime_error ("Error when deserializing: wrong size of frequencies array");
		}
		cumulative_probs.resize(KMERS * CHARACTERS);
	}
};

};

#endif
