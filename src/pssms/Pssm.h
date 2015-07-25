/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/


#ifndef Pssm_h__
#define Pssm_h__

#include <string>
#include <vector>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <fstream>

#include <algorithm>
#include <limits>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "bsp.h"

#include "datamodel/Serializable.h"
#include "datamodel/TextIO.h"
#include "datamodel/SequenceTranslation.h"
#include "sequencemodel/SequenceModel.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <time.h>

namespace pssms {

	template <size_t alphasize = 4>
	class PSSM : public datamodel::Serializable {
	public:

		enum { 
			ALPHASIZE = alphasize, 
		};

		/**
		 * Default constructor. 
		 * 
		 * Initialize background distribution
		 * 
		 */
		void defaults() {
			// DNA?
			if (alphasize == 4) {
				character_translation = "ACGT";
				// default background distribution for default 
				// Sequence translation ACGT -> 0123
				background[0] = 0.3;		// A 
				background[1] = 0.2;		// C 
				background[2] = 0.2;		// G 
				background[3] = 0.3;		// T 
			} else {
				character_translation.resize(alphasize);
				for (int j = 0; j < alphasize; ++j) {
					background[j] = 1.0 / alphasize;
					character_translation [j] = 'A' + j;
				}
			}
			set_length(0);

			for (int i = 0; i < length; ++i) {
				for (int j = 0; j < alphasize; ++j) {
					(*this) (i, j) = background[j];
				}
			}

			name = "BLANKPSSM";
			offset = 0;
		}

		/**
		 * Assignment
		 */
		PSSM<alphasize> & operator=  (PSSM<alphasize> const & rhs) {
			memcpy (background, rhs.background, sizeof (double) * alphasize);
			length = rhs.length;
			matrix = rhs.matrix;
			name = rhs.name;
			nsites = rhs.nsites;
			accession = rhs.accession;
			character_translation = rhs.character_translation;
			offset = rhs.offset;
			pseudocount = rhs.pseudocount;
			return *this;
		}

		/**
		 *	Get and set length.
		 */
		int get_length() const { return (int)length; }
		void set_length(size_t val) { 
			matrix.resize(val * alphasize);
			length = (int)val; 
		}

		/**
		 * Get/set name
		 */
		std::string get_name () const {
			return name;
		}
		void set_name (std::string const & n) {
			name = n;
		}

		/**
		 * Shift left by num positions
		 */
		PSSM<alphasize> & shift_left (int num = 1) {
			offset+= num;
			return *this;
		}

		/**
		 * Shift right by num positions
		 */
		PSSM<alphasize> & shift_right (int num = 1) {
			offset-= num;
			return *this;
		}

		PSSM<alphasize> & reset_shift () {
			offset = 0;
			return *this;
		}

		/**
		 * first and last bp
		 */
		int begin () const {
			return -offset;
		}

		/**
		 * first and last bp
		 */
		int end () const {
			return length - offset - 1;
		}

		/**
		 * reverse location
		 */
		virtual int reverse_pos (int pos) const {
			return end() - ( pos - begin () );
		}

		/**
		 * reverse character
		 */
		virtual int reverse_char (int c) const {
			return alphasize - c - 1;
		}

		/**
		 * Get probabilities
		 */
		const double & operator () (int _pos, int _char) const {
			int pos = _pos + offset;
			ASSERT (_char >= 0 && _char < alphasize);

			if (pos >=0 && pos < length) {
				return matrix[pos*alphasize + _char];
			} else {
				return background[_char];
			}
		}
/*		
 *		TODO : if we'd want to set PSSM values, this would be the way to go,
 *		however, we might want to include dynamic resizing too.
 */
 		double & operator () (int _pos, int _char) {
			int pos = _pos + offset;
			ASSERT (_char >= 0 && _char < alphasize);

			if (pos >= 0 && pos < length) {
				return matrix[pos*alphasize + _char];
			} else {
				throw std::runtime_error ("Cannot set probability at position outside PSSM range.");
			}
		}
		
		/**
		 * Calculate entropy for a specified PSSM. 
		 *  (sum of individual entropy values)
		 *
		 * @return Returns entropy of a PSSM.
		 */
		double entropy () {
			double entropy = 0;
			static double log2 = log(2.0);

			for(int i = 0; i < length; i++) {
				for (int j = 0; j < alphasize; ++j) {
					double v = (*this)(i, j);
					if (v > FLT_MIN) {
						entropy -= (*this)(i, j) * log( (*this)(i, j) ) / log2;
					}
				}
			}

			return entropy;
		}

		/** load from JASPAR matrix_only.txt file
		 * Ex: 
		 * >MA0001.1 AGL3
		 *	A  [ 0  3 79 40 66 48 65 11 65  0 ]
		 * 	C  [94 75  4  3  1  2  5  2  3  3 ]
		 * 	G  [ 1  0  3  4  1  0  5  3 28 88 ]
		 *	T  [ 2 19 11 50 29 47 22 81  1  6 ]
		 */
		std::istream & load_jaspar_pssm(std::istream & in) {
			ASSERT (alphasize == 4);
			using namespace std;
			bool foundone = false;
			string current_line = "";
			while (in.good()) {
				if (getline(in, current_line, '\n')) {
					if(current_line.at(0) == '>') {
						foundone = true;
						break;
					}
				}
			}
			if(!foundone) {
				throw std::runtime_error("Error: can't read JASPAR data file (no name found).\n");
			}
			current_line = TextIO::trim(current_line, "\n\r \t>");
			size_t sp = current_line.find(" ");
			if (sp != string::npos) {
				accession = current_line.substr(0, sp);
				name = current_line.substr(sp+1);
			} else {
				accession = name = current_line;
			}
			pseudocount = 0.25;

			int chars = 4;
			int len = -1;
			bool has_char = true;
			while (in.good() && chars > 0) {
				if (getline(in, current_line, '\n')) {
					current_line = TextIO::trim(current_line);
					char whichone = current_line.at(0);
					int index = 0;
					switch(whichone) {
						case 'A':
							index = 0;
							break;
						case 'C':
							index = 1;
							break;
						case 'G':
							index = 2;
							break;
						case 'T':
							index = 3;
							break;
						default:
							has_char = false;
							index = 4 - chars;
							break;
					}
					size_t pp1 = current_line.find('[');
					size_t pp2 = current_line.find(']');

					if (!has_char)
					{
						pp1 = 0;
						pp2 = current_line.size();
					}
					else
					{
						if (pp1 == string::npos || pp2 == string::npos || pp2 < pp1 + 2) {
							throw std::runtime_error("Error: can't read JASPAR data file: can't find '['/']'.\n");
						}
						current_line = current_line.substr(pp1+1, pp2-pp1-1);
					}

					std::vector<std::string> v;
					TextIO::split(current_line, v, " \t", " ", true);
					if (len < 0) {
						len = (int)v.size();
						set_length(v.size());
					} else {
						if (len != (int)v.size()) {
							std::ostringstream oss;
							oss << "Error: can't read JASPAR data file: inconsistent row lengths: " << len << " " << v.size() << std::endl;
							throw std::runtime_error(oss.str().c_str());
						}
					}
					int i = 0;
					for (std::vector<std::string>::iterator ix = v.begin(); ix != v.end(); ++ix) {
						(*this)(i, index) = atoi(ix->c_str());
						++i;
					}
					chars--;
				}
			}

			// normalize and figure out nsites
			nsites = -1;
			for (int i = 0; i < length; ++i) {
				double ns_i = 0;
				for (int j = 0; j < alphasize; ++ j) {
					ns_i = ns_i + (*this)(i, j);
				}
				if (nsites < 0) {
					nsites = std::max((int)ns_i, nsites);
				}
				for (int j = 0; j < alphasize; ++ j) {
					(*this)(i, j)/= ns_i;
				}
			}

			if (nsites < 0)
			{
				throw std::runtime_error("Error: can't read JASPAR data file: inconsistent nsites.\n");
			}

			return in;
		}

		/**
		 * Input old format where we store PSSM's like this:
		 * 
		 * NAME LENGTH
		 * p_1 (a) p_1 (c) p_1 (g) p_1 (t) 
		 * p_2 (a) p_2 (c) p_2 (g) p_2 (t) 
		 * ...
	     * 
		 */

		void load_custom_pssm (const char * filename) {
			ASSERT (alphasize == 4);
			std::ifstream f (filename);

			if(!f.good()) {
				throw std::runtime_error("Error: can't open input file.\n");
			}
			load_custom_pssm(f);
		}


		void load_custom_pssm (std::istream & f) {
			ASSERT (alphasize == 4);
			using namespace std;
			defaults();
			f >> name >> length;
			if (length <= 0) {
				throw runtime_error("Error: PSSM cannot have zero length.");
			}
			set_length(length);
			
			for(int p = 0; p < length; p++) {
				for (int c = 0; c < alphasize; ++c ) {
					f >> matrix[p*alphasize + c];
				}
			}
		}

		/**
		 * Randomly generate sequence and insert into a string
		 */
		template <class sequence>
		void random_sequence (sequence & b, int offset = 0) {
			if (b.size() < offset + length) {
				b.resize(offset + length);
			}

			int k = offset;
			for (int i = begin(); i <= end(); ++i) {
				double d = random();
				double v = 0;

				int c = ALPHASIZE - 1;

				for (int j = 0; j < ALPHASIZE; ++j) {
					v+= (*this)(i, j);
					if (d < v) {
						c = j;
						break;
					}
				}
				b[k++] = c;
			}
		}

		typedef enum {
			PSEUDOCOUNT_NONE,
			PSEUDOCOUNT_BIFA,
			PSEUDOCOUNT_LINEAR,
			PSEUDOCOUNT_SQRT,
		} pseudocount_t;

		/**
		 * Add a pseudocount and normalize
		 */
		void add_pseudocount (pseudocount_t type = PSEUDOCOUNT_BIFA) {
			double pc = 0;
			switch (type) {
			case PSEUDOCOUNT_LINEAR:
				pc = 1.0 / (4*sqrt ((double)nsites));
				break;
			case PSEUDOCOUNT_SQRT:
				pc = 1.0 / (2*nsites);
				break;
			case PSEUDOCOUNT_BIFA:
			default:
				pc = pseudocount;
				break;
			case PSEUDOCOUNT_NONE:
				pc = DBL_EPSILON;	// not quite none, but close
				break;
			}

			for (int k = begin(); k <= end(); ++k) {
				double sum = 0;
				for (int l = 0; l < alphasize; ++l) {
					(*this)(k, l)+= pc;
					sum+= (*this)(k, l);
				}
				for (int l = 0; l < alphasize; ++l) {
					(*this)(k, l) /= sum;
				}
			}
		}


		double get_pseudocount() { 
			if (pseudocount < DBL_EPSILON) {
				return 1.0 / sqrt ((double)length);
			} else {
				return pseudocount;
			}
		}

		void set_pseudocount(double val) { 
			pseudocount = val; 
		}
		
		std::string get_accession() const {
			return accession;
		}

	protected:
	
	/**
	 * (static) Buffered random number generation
	 */
	static double random () {
		static bool init = false;
		static size_t  rng_pos = 1024;
		static boost::random::mt19937 rng_state;
		static boost::random::uniform_real_distribution< > rng_dist( 0.0, 1.0 );
		static boost::variate_generator< boost::mt19937&, boost::random::uniform_real_distribution < > >
        		generateRandomNumbers( rng_state, rng_dist );
		static double  rng_buffer [1024];

		if (!init) {
			init = true;
			rng_state.seed(time(0));
		}

		if (rng_pos >= 1024) {
			for (int i = 0; i < 1024; ++i)
			{
				rng_buffer[i] = generateRandomNumbers();
			}
			rng_pos = 0;
		}

		return rng_buffer[rng_pos++];
	}

	private:
		/************************************************************************/
		/* Serialization code                                                   */
		/************************************************************************/
		JSONIZE_AS (
			"Datatypes::Motifs::PSSM", 
			PSSM<alphasize>, 1, 
			S_STORE(name, JSONString<>)
			S_STORE(accession, JSONString<>)
			S_STORE(character_translation, JSONString<>)
			S_STORE(length, JSONInt<>)
			S_STORE(offset, JSONInt<>)
			S_STORE(nsites, JSONInt<>)
			S_STORE(pseudocount, JSONDouble<>)
			S_STORE(background, JSONStaticArray< JSONDouble<>, alphasize >)
			S_STORE(matrix, JSONArray< JSONDouble<> >)
		);

		std::vector<double> matrix;

		std::string name;
		std::string accession;
		std::string character_translation;

		int length;
		int offset;
		int nsites;

		double pseudocount;
		double background[alphasize];
	};

	/************************************************************************/
	/* Score function base class                                            */
	/************************************************************************/
	template <class string>
	class PSSM_Score {
	public:
		virtual ~PSSM_Score() {}
		virtual double min_score() = 0;
		virtual double max_score() = 0;

		virtual double score (string const & sequence, int offset = 0) = 0;
		virtual size_t min_length() = 0;
	};

	// distance functors are in here.	#include "Pssm_distance.inl"
	#include "Pssm_distance.inl"
	#include "Pssm_multiplicative.inl"
	#include "Pssm_profile.inl"

	// naive score factory. this could probably done better.
	template <class string, size_t alphasize = 4> 
	class PSSM_Score_Factory {
	public:
		enum scoretype_t {
			MULT_NO_PC,
			MULT_SQRT_PC,
			MULT_LINEAR_PC,
			MULT_BIFA_PC
		} ;

		typedef boost::shared_ptr<PSSM_Score < string > > product_t;

		/**
		 * string to score type expansion.
		 */

		static scoretype_t s (const char * st) {
			if (st == NULL) {
				throw std::runtime_error("Unknown score type.");
			}

			if (strstr(st, "mult") == st) {

				if (strstr(st, "no") != NULL) {
					return MULT_NO_PC;
				}

				if (strstr(st, "linear") != NULL) {
					return MULT_LINEAR_PC;
				}

				if (strstr(st, "sqrt") != NULL) {
					return MULT_SQRT_PC;
				}

				if (strstr(st, "bifa") != NULL) {
					return MULT_BIFA_PC;
				}
			}

			throw std::runtime_error("Unknown score type.");
		}

		/**
		 * Create a score object from a PSSM.
		 */
		static boost::shared_ptr< PSSM_Score <string> > create ( PSSM<alphasize> const & p, scoretype_t scoretype ) {
			switch ( scoretype ) {
			case MULT_NO_PC:
				{
					boost::shared_ptr< PSSM_Score <string> > ret (new PSSM_Multiplicative_Score < string, alphasize > (p, PSSM< alphasize > :: PSEUDOCOUNT_NONE ));
					return ret;
				}
			case MULT_SQRT_PC:
				{
					boost::shared_ptr< PSSM_Score <string> > ret (new PSSM_Multiplicative_Score < string, alphasize > (p, PSSM< alphasize > :: PSEUDOCOUNT_SQRT ));
					return ret;
				}
			case MULT_LINEAR_PC:
				{
					boost::shared_ptr< PSSM_Score <string> > ret (new PSSM_Multiplicative_Score < string, alphasize > (p, PSSM< alphasize > :: PSEUDOCOUNT_LINEAR ));
					return ret;
				}
			case MULT_BIFA_PC:
				{
					boost::shared_ptr< PSSM_Score <string> > ret (new PSSM_Multiplicative_Score < string, alphasize > (p, PSSM< alphasize > :: PSEUDOCOUNT_BIFA ));
					return ret;
				}
			default:
				throw std::runtime_error("Unknown score type.");
			};
		}
	};

};

#endif // Pssm_h__
