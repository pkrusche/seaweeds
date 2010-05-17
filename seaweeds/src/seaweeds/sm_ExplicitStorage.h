/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __SM_EXPLICITSTORAGE_H__
#define __SM_EXPLICITSTORAGE_H__

#include <algorithm>
#include <boost/multi_array.hpp>

#include "lcs/Llcs.h"

namespace seaweeds {

template <class _lcsfun> 
class ExplicitStorage {
public:
	typedef boost::multi_array<int, 2> array_type;
	typedef typename array_type::index index;
	typedef typename _lcsfun::string string;
	typedef array_type archive;

	ExplicitStorage(int _m, int _n) : A(boost::extents[_m+_n][_m+_n]), m(_m), n(_n) {}
	ExplicitStorage(int _m, int _n, archive const & a) : A(a), m(_m), n(_n) {}

	/**
	 * \brief Returns whether a given pair of coordinates is in the core
	 * 
	 * \param i 
	 * \param j 
	 * \return 
	 */
	bool nontrivial(int i, int j) {
		return -m <= i && i < n && 0 <= j && j < m+n;
	}

    /**
     * @brief query density matrix at points (i, j)
     *
     * @return int the number of points dominated by ints (i,j)
     *
     */
    int distribution(int i, int j) {
		/* check if nontrivial */
		if( nontrivial(i, j) ) {
        	return A[i+m][j];
		} else {
			/* otherwise we only have trivial nonzeros in the density matrix */
			if(j > i+m)
				return j - i - m;
			else 
				return 0;
		}
    }

	/**
	 * \brief density matrix. odd half ints are mapped to ints as floors.
	 * \param ihat 
	 * \param jhat 
	 * \return 
	 */
	int density(int ihat, int jhat) {
		return distribution(ihat-1, jhat+1) + distribution(ihat, jhat) - distribution(ihat-1, jhat) - distribution(ihat, jhat+1);
	}

	/**
	 * \brief initialize density matrix.
	 */
	void identity() {
		using namespace std;
		for(int i = -m; i < n; ++i) {
			for(int j = 0; j < m+n; ++j) {
				A[i+m][j] = max(0, (int)(j - i - m));
			}
		}
	}

	/**
	 * \brief initialize as semi-local scores corresponding to comparing s1 and s2
	 * This is a naive reference implementation. Time O(n^4)
	 * \param s1 string alongside the vertical edge of the alignment dag
	 * \param s2 string alongside the horizontal edge of the alignment dag
	 */
	void semilocallcs(string s1, string s2) {
		using namespace std;

		x = s1;
		y = s2;
		m = s1.size();
		n = s2.size();
		A.resize(boost::extents[m+n][m+n]);
		_lcsfun lcs;
		for(int i = -m; i < n; ++i) {
			for(int j = 0; j < m+n; ++j) {
				int value = 0;
				if(j < n) {
					if(i < 0) {
						// suffix-prefix lcs
						int suflen = m+i;
						int prelen =  j;
						typename _lcsfun::string 
							l1(s1.substr(m-suflen, suflen)) ;
						typename _lcsfun::string 
							l2(s2.substr(0, prelen)) ;

						// prefix+suffix + length of path to get to start of suffix
						// in alignment dag
						value = (int)lcs(l1, l2) + m - suflen;
					} else { // i>= 0
						// string-substring lcs
						int pos = i;
						int len = j - i;
						if(len > 0) {
							typename _lcsfun::string 
								l1(s2.substr(pos, len)) ;
							value = (int)lcs(l1, s1);
						} else {
							value = len;
						}
					} 
				} else { // j >= n
					if(i < 0) {
						// substring-string lcs
						int pos = -i;
						int len = m - (j - n) - pos;
						if(len > 0) {
							typename _lcsfun::string 
								l1(s1.substr(pos, len)) ;
							value = min(m, (int)lcs(l1, s2) + pos + j - n);
						} else {
							value = min(m, pos + j - n);
						}
					} else { // i >= 0
						// prefix-suffix lcs
						int prelen =  m+n-j;
						int suflen = n-i;
						typename _lcsfun::string 
							l1(s1.substr(0, prelen)) ;
						typename _lcsfun::string 
							l2(s2.substr(i, suflen)) ;

						// prefix+suffix + length of path to get to start of suffix
						// in alignment dag
						value = (int)lcs(l1, l2) + j - n;
					}
				}
				// this is to make it fit into the highest-score matrix
				A[i+m][j] = j - i - value;
			}
		}
	}

	/**
	 * \brief Compute the matrix for the reverse of x and y in O(m+n) time.
	 */
	void reverse_xy() {
		using namespace std;
		reverse(x.begin(), x.end());
		reverse(y.begin(), y.end());
		semilocallcs(x, y);
	}

	typedef enum _incremental_type {
		APPEND_TO_X, 
		APPEND_TO_Y, 
	} incremental_type;

	/**
	* \brief Incremental LCS computation.
	*/
	void incremental_semilocallcs(const string & s, incremental_type t = APPEND_TO_X) {
		switch(t) {
			case APPEND_TO_X: 
				x.append(s);
				break;
			case APPEND_TO_Y:
				y.append(s);
				break;
		}
		semilocallcs(x,y);
	}
/*

	ExplicitStorage * highest_score_composition(ExplicitStorage const & _B, int offset) {
		array_type & B (_B.A);
		array_type C(boost::extents[new_m+new_n][new_m+new_n]);
		
		for (int j = 0; j < new_m+new_n; ++j) {

		}
		
	}
*/

	archive & get_archive() {
		return A;
	}

	int get_m() {
		return m;
	}

	int get_n() {
		return n;
	}

	string const & get_x() {
		return x;
	}

	string const & get_y() {
		return y;
	}

	/**
	 * @brief Query scores for all windows in y that have a given fixed length
	 * @return the number of complete matches (windows that contain the full pattern as a subsequence)
	 */
	template <class _report>
	size_t query_y_windows(size_t windowlength, size_t stepsize, _report * rpt) {
		size_t count = 0;

		// naive approach by repeated score query
		for (size_t j = 0; j <= n - windowlength; ++j) {
			int score = windowlength - distribution(j, j+windowlength);
			if(score == m) {
				++count;
			}
			if(rpt != NULL && (j & (stepsize - 1)) == 0) {
				(*rpt)(j, score);
			}
		}
		return count;
	}

protected:
	string x, y;
	int m, n; ///< The dimensions of the core.
	array_type A;
};

};

#endif
