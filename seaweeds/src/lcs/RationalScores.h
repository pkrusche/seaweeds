/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __RATIONALSCORES_H__
#define __RATIONALSCORES_H__


namespace lcs {
/**
 * \date 02-04-2009
 *
 * \author peter
 * 
 * This code implements arbitrary rational scores on top of LCS computation 
 * by using a constant size input blowup.
 * 
 * \todo 
 *
 * \bug 
 *
 */
template<class _string, int _cellsize = 2, int _mismatches = 1>
class ScoreTranslation{
public:
	typedef _string string;

	enum {
		cellsize = 2,
		diag_mismatches = 1,
	};

	/**
	 * \brief This functions blows up an input string. 
	 * \param input 
	 * \return 
	 */
	static string translate(string const & input) {
		string s;
		s.resize(input.size()*cellsize);
		// add new character "1" by incrementing every input char
		for(size_t j = 0; j < input.size(); ++j) {
			for(size_t k = 0; k < cellsize; ++k) {
				if(k < diag_mismatches) {
					s[j*cellsize + k] = 1;
				} else if (s[j] != 0) {
					s[j*cellsize + k] = input[j];
				}
			}
		}
		return s;
	}

	/**
	 * \brief Translate scores obtained using blown up input.
	 * \param m length of original input string one
	 * \param n length of original input string two
	 * \param score LCS score that was obtained
	 * \return the score according to the rational weighting scheme
	 */
	static double translatescore(size_t m, size_t n, double score) {
		return ((double)score) - 0.5*m - 0.5*n ;
	}

	/**
	* \brief Translate scores for normal input into blown up scores.
	* \param m length of original input string one
	* \param n length of original input string two
	* \param score LCS score according to the rational weighting scheme
	* \return the score in the blown up alignment dag
	*/
	static double translatescore_fwd(size_t m, size_t n, double score) {
		return ((double)score) + 0.5*m + 0.5*n ;
	}

	/**
	 * \brief translate coordinates/positions from original to blown up version
	 * \param coord 
	 * \return 
	 */
	static size_t translatecoord_fwd(size_t coord) {
		return coord*cellsize;
	}

	/**
	 * \brief translate coordinates/positions back to original
	 * \param coord 
	 * \return 
	 */
	static size_t translatecoord_bk(size_t coord) {
		return coord/cellsize;
	}


	/**
	 * \brief check if coordinate translates to a valid or a fractional position in the original string
	 * \param coord 
	 * \return 
	*/
	static bool validcoord(size_t coord) {
		return coord % cellsize == 0;
	}
};

};

#endif
