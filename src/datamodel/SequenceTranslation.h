/*
 * SequenceTranslation.h
 *
 *  Created on: 9 Jan 2012
 *      Author: peterkrusche
 */

#ifndef SEQUENCETRANSLATION_H_
#define SEQUENCETRANSLATION_H_

#include <string>
#include <stdexcept>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/grep.hpp>
#include <boost/iostreams/filter/regex.hpp>

#include "xasmlib/IntegerVector.h"


#include "SequenceStream.h"

namespace datamodel {

/**
 * @brief translate a bit-encoded sequence to a string
 *
 * @parameter a : the sequence
 * @parameter chars : the allowed characters
 * @parameter unknown_char : what to insert if we don't have enough characters
 *
 * @return an integer vector containing the translated string
 */
template<size_t _bpc>
std::string unwrap_sequence(utilities::IntegerVector<_bpc> const & v,
		std::string chars = "ACGT", char unknown_char = 'N') {
	using namespace std;
	size_t len = chars.size();
	string seq;
	seq.resize(v.size());
	for (size_t j = 0; j < v.size(); ++j) {
		UINT64 val = v.get(j);
		if (val < len) {
			seq[j] = chars[val];
		} else {
			seq[j] = unknown_char;
		}
	}

	return seq;
}

template <size_t bpc_in, size_t bpc_out>
void transliterate (
	utilities::IntegerVector<bpc_in> const & in,
	utilities::IntegerVector<bpc_out>      & out, 
	std::string alpha_in = "ACGT",  std::string alpha_out = "ACGT") {
	ASSERT ( (1 << bpc_in) >= alpha_in.size() );
	ASSERT ( (1 << bpc_out) >= alpha_out.size() );
	out.resize(in.size());

	int translit_table[1 << bpc_in];

	memset ( translit_table, 0xff, sizeof (int) * (1 << bpc_in) );

	for (size_t k = 0; k < alpha_in.size(); ++k) {
		for (size_t l = 0; l < alpha_out.size(); ++l) {
			if (alpha_in[k] == alpha_out[l]) {
				translit_table[k] = (int) l;
			}
		}
	}

	for (int j = 0; j < (int) in.size(); ++j) {
		out.put (j, translit_table[in.get(j)]);
	}
}

template <typename charbuffer, size_t bpc> 
void transliterate_string (charbuffer const & buffer, size_t len, 
	utilities::IntegerVector<bpc> & target, std::string translation = "ACGT") {
	if (target.size() < len) {
		target.resize(len);
	}
	for (size_t j = 0; j < len; ++j) {
		char c = buffer[(int)j];
		int kk = 0;
		bool found = false;
		for (size_t k = 0; k < translation.size(); ++k) {
			if (c == translation[k]) {
				kk = (int)k;
				found = true;
				break;
			}
		}
		if (!found) {
			throw std::runtime_error(std::string("Untranslatable character'") + std::string(&c, 1) + std::string ("' found in input."));
		}
		target.put((int)j, kk);
	}
}

/**
 * @brief translate a string through alphabet translation into a bit-encoded sequence
 *
 * @parameter a : the string
 * @parameter chars : the allowed characters
 *
 * @return an integer vector containing the translated string
 */
template<size_t _bpc>
utilities::IntegerVector<_bpc> make_sequence(const char* _a,
		std::string chars = "ACGT") {
	utilities::IntegerVector<_bpc> s;
	transliterate_string<const char*, _bpc>(_a, strlen(_a), s, chars);
	return s;
}

/**
 * @brief reverse-complement a string.
 * 
 * v[i] = (c - 1 - v[v.size() - 1 - i])
 *
 * @parameter v the string
 * @parameter c maximum value + 1 to mirror on (v[i] \in {0,1,2,3} => c = 4)
 *
 */
template<size_t _bpc>
void reverse_complement(utilities::IntegerVector<_bpc> & v, int c) {
	UINT64 cc = c - 1;
	
	for (size_t i = 0; i < v.size(); ++i) {
		if (v[i] <= cc) {
			v[i] = cc - v[i];			
		}
	}
	v.reverse();
}

/** A sequence input source which translates a character input stream
 *  and transliterates it into binary. */
template <size_t bpc> 
class TranslatingInputStream : public SequenceSource<bpc> {
public:

	TranslatingInputStream(std::istream & input, 
		const char * _translation = "ACGT"
	) {
		translation = _translation;
		ASSERT( (1l << bpc)  >= strlen (_translation) );
		in.push (input);
	}

	/**
	 * Read sequence into target integer vector
	 */
	virtual size_t read_sequence (utilities::IntegerVector <bpc> & target) {
		if (target.size() == 0) {
			return 0;
		}
		if ( buffer.exact_size() < target.size()) {
			buffer.resize( target.size() );
		}
		size_t len = 0;
		for (int k = 0; k < target.size(); ++k) {
			int input;
			do {				
				input = in.get();
			} while (input <= ' ' && in.good());
			
			if (input > ' ' && input < 256) {
				buffer[k] = input;
				++len;
			}
			if (!in.good()) {
				break;
			}
		}

		target.one();
		transliterate_string<utilities::AVector<char>, bpc> (buffer, len, target, translation);
		return len;
	}

private:
	boost::iostreams::filtering_istream in;
	std::string translation;
	utilities::AVector<char> buffer;
};

}
;

#endif /* SEQUENCETRANSLATION_H_ */
