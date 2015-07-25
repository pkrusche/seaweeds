/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef __SM_IMPLICITSTORAGE_H__
#define __SM_IMPLICITSTORAGE_H__

#include <algorithm>
#include <boost/smart_ptr.hpp>

#include "rangesearching/Range2D.h"
#include "seaweeds/Seaweeds.h"

namespace seaweeds {

/**
 *  \brief Implicit highest score matrices using the column-based seaweed algorithm
 */
template <class _slcsfun = seaweeds::Seaweeds<>, 
	      template <class, class> class range = rangesearching::Range2DTL
>
class ImplicitStorage {
public:
	typedef range<int, functors::count<rangesearching::Point2D<int> > > _rangetree;
	typedef rangesearching::Point2D<int> _point;
	typedef typename _slcsfun::string string;
	typedef std::vector<int> archive;

	ImplicitStorage (int _m, int _n) : m(_m), n(_n), seaweedpermutation() /*, rangetree(NULL) */ {
		ensure_sizes(m, n);
	}

	ImplicitStorage (int _m, int _n, archive const & a) : m(_m), n(_n), seaweedpermutation(a) /*, rangetree(NULL) */ {
		ensure_sizes(m, n);
	}

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
			if(rangetree == NULL) {
				buildrangetree();
			}
			rangesearching::Point2D<int> topleft(i+m+1, 0);
			rangesearching::Point2D<int> bottomright(m+n, j);
			size_t zero = 0;
        	return (int)rangetree->query(topleft, bottomright, zero);
		} else {
			/* otherwise we only have trivial seaweedpermutation in the density matrix */
			if(j > i + m)
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
		if( (ihat+m-1) < (signed)m+n && (signed)ihat+m-1 >= 0) {
			return (jhat == seaweedpermutation[ihat+m-1]) ? 1 : 0;
			//  use this to query density using rangetree (O(log^2 n) instead of O(1))
			//	distribution(ihat-1, jhat+1) + distribution(ihat, jhat) - distribution(ihat-1, jhat) - distribution(ihat, jhat+1);
		} else {
			return jhat == ihat+m-1;
		}
	}

	/**
	 * \brief initialize density matrix.
	 */
	void identity() {
		using namespace std;
		for(int i = 0; i < m+n; ++i) {
			seaweedpermutation[i] = i;
		}
		x.resize(m);
		for(int i = 0; i < m; ++i) {
			x[i] = 0;
		}
		y.resize(n);
		for(int i = 0; i < n; ++i) {
			y[i] = 0;
		}
	}

	/**
	 * \brief initialize as semi-local scores corresponding to comparing s1 and s2
	 * 
	 * \param s1 string alongside the vertical edge of the alignment dag
	 * \param s2 string alongside the horizontal edge of the alignment dag
	 */
	void semilocallcs(const string & s1, const string & s2) {
		using namespace std;

		x = s1;
		y = s2;
		m = (int)s1.size();
		n = (int)s2.size();
		ensure_sizes(m,n);
		rangetree = boost::shared_ptr<_rangetree> ();

		_slcsfun f;
		f(s1, s2, right, top, false, false);
		seaweed_distances_to_permutation();
	}

	typedef enum _incremental_type {
		APPEND_TO_X, 
		APPEND_TO_Y, 
	} incremental_type;

	/**
	 * \brief Incremental LCS computation.
	 */
	void incremental_semilocallcs(const string & s, incremental_type t = APPEND_TO_X) {
		using namespace std;
		_slcsfun f;
		size_t mm = m+n;
		switch(t) {
			case APPEND_TO_X: 
			{
				x.append(s);
				
				ensure_sizes(m+(int)s.size(), n);

				for(size_t t = 0; t < s.size(); ++t) {
					right[t] = t + m + 1;
				}
				for(size_t t = 0; t < mm; ++t) {
					int j = seaweedpermutation[t];
					if (j >= 0 && j < n) {
						top[j] = j - t + m;
					}
				}

				// extend seaweeds
				f(s, y, right, top, true, true);

				m+= (int)s.size();
				ensure_sizes(m, n);

				// restore seaweed permutation from distances
				for(long int j = (long int)(m+n-s.size()) - 1; j >= 0 ; --j) {
					long int  z = seaweedpermutation[j];
					if(z >= n) {
						long int i = j+(long int)s.size();
						z+= (long int)s.size();
						seaweedpermutation[i] = z;
#ifdef _DEBUG_SEAWEEDS
						cout << i - (long int)m << " -> " << z  << endl;
#endif // _DEBUG_SEAWEEDS
					}
				}
#ifdef _DEBUG_SEAWEEDS
				cout << endl;
#endif // _DEBUG_SEAWEEDS
				for (size_t j = 0; j < s.size(); ++j) {
					int v = right.get(j);
#ifdef _DEBUG_SEAWEEDS
					cout << "right [" << j << "] = " << v << endl;
#endif // _DEBUG_SEAWEEDS
					if(v < _slcsfun::permutation_container::lsbs) {
						seaweedpermutation[n - v - 1 + m] = 
							(int) (s.size() + n - j - 1);
					}
#ifdef _DEBUG_SEAWEEDS
					cout << (int)(n - v - 1) << " -> " 
						 << (int)(s.size() + n - j - 1) << endl;
#endif // _DEBUG_SEAWEEDS
				}
				for (size_t j = 0; j < n; ++j) {
					int v = top.get(j);
					if(v < _slcsfun::permutation_container::lsbs) {
						seaweedpermutation[j - v + m] = (int)j;
					}
#ifdef _DEBUG_SEAWEEDS
					cout << (int)(j - v) << " -> " << (int)j << endl;
#endif // _DEBUG_SEAWEEDS
				}

				rangetree = boost::shared_ptr<_rangetree > ();
				break;
			}
			case APPEND_TO_Y: 
			{
				y.append(s);
				ensure_sizes(m, n+(int)s.size());
				
				for(size_t t = 0; t < mm; ++t) {
					int j = seaweedpermutation[t];
					if (j >= n) {
						size_t k = m - (t - n);
						right[m+n-j-1] = k < _slcsfun::permutation_container::lsbs ? k 
													: _slcsfun::permutation_container::lsbs;
#ifdef _DEBUG_SEAWEEDS
						cout << "right [" << m+n-j-1 << "] = " << k << endl;
#endif // _DEBUG_SEAWEEDS
					}
				}

				f(x, s, right, top, true, false);

				n+= (int)s.size();

#ifdef _DEBUG_SEAWEEDS
				// Debug output. The seaweeds which reach the bottom before the
				// appended string remain the same
				for(int j = 0; j < m+n-(int)s.size(); ++j) {
					int z = seaweedpermutation[j];
					if(z < n-(int)s.size()) {
						cout << (signed)j - m << " -> " << (signed)z  << endl;
					}
				}
				cout << endl;
#endif // _DEBUG_SEAWEEDS
				// These are the new seaweeds that reach the right side
				for (int j = 0; j < m; ++j) {
					int v = right.get(j);
#ifdef _DEBUG_SEAWEEDS
					cout << "right [" << j << "] = " << v << endl;
#endif // _DEBUG_SEAWEEDS
					if(v < _slcsfun::permutation_container::lsbs) {
						size_t i = n - v - 1 + m;
						size_t k = m + n - j - 1;
						seaweedpermutation[i] = (int)k;
#ifdef _DEBUG_SEAWEEDS
						cout << i-m << " -> " << k << endl;
#endif // _DEBUG_SEAWEEDS
					}
				}
#ifdef _DEBUG_SEAWEEDS
				cout << endl;
#endif // _DEBUG_SEAWEEDS
				for (int j = 0; (unsigned)j < s.size(); ++j) {
					int v = top.get(j);
					if(v < _slcsfun::permutation_container::lsbs) {
						seaweedpermutation[j + n - v + m - s.size()] = 
							(int) (j + n - s.size());
					}
#ifdef _DEBUG_SEAWEEDS
					cout << (int)(j + n - v - s.size() ) << " -> " << (int)j + n - s.size() << endl;
#endif // _DEBUG_SEAWEEDS
				}

				rangetree = boost::shared_ptr <_rangetree> ();
				break;
			}
		}
	}

	/**
	 * \brief Compute the matrix for the reversed strings x' and y' in O(m+n) time.
	 */
	void reverse_xy() {
		/*
		 * reverse, invert, reverse.
		 */
		using namespace std;
		x.reverse();
		y.reverse();
		std::vector<int> new_nonzeros;
		int init = -1;
		size_t mm = m+n;

		new_nonzeros.resize(mm, init);		
		size_t j = 0;
		for (j = 0; j < mm/2; ++j) {
			swap(seaweedpermutation[j], seaweedpermutation[mm-j-1]);
		}
		for (j = 0; j < mm; ++j) {
			if (seaweedpermutation[j] >= 0) {
				new_nonzeros[seaweedpermutation[j]] = (int)j;
			}
		}
		for (j = 0; j < mm; ++j) {
			seaweedpermutation[j] = new_nonzeros[mm -j - 1];
		}
		rangetree = boost::shared_ptr<_rangetree> ();
	}

	/**
	 * \brief initialize the range searching data structure
	 */
	void buildrangetree() {
		typename _rangetree::container_t container;
		for (int j = 0; j < m+n; ++j) {
			// in restricted highest-scorematrices (subpermutations), we identify
			// the elements we don't know by -1
			if(seaweedpermutation[j] != -1) {
				container.insert(container.end(), _point(j+1,seaweedpermutation[j]+1));
			}
		}
		rangetree = boost::shared_ptr<_rangetree> (new _rangetree(container));
	}

	archive & get_archive() {
		return seaweedpermutation;
	}

	size_t get_m() {
		return m;
	}

	size_t get_n() {
		return n;
	}

	string & get_x() {
		return x;
	}

	string & get_y() {
		return y;
	}

	bool equals(ImplicitStorage<_slcsfun, range> const & s) {
		using namespace std;
		if(m != s.m || n != s.n || x.size() != s.x.size() || y.size() != s.y.size()) {
			return false;
		}

		for (int j=0; j < m+n; ++j) {
			if(seaweedpermutation[j] != s.seaweedpermutation[j]) {
				return false;
			}
		}
		for (int j=0; j < x.size(); ++j) {
			if(x[j] != s.x[j]) {
				return false;
			}
		}
		for (int j=0; j < y.size(); ++j) {
			if(y[j] != s.y[j]) {
				return false;
			}
		}
		
		return true;
	}

	/**
	 * @brief Query scores for all windows in y that have a given fixed length
	 * @return the number of complete matches (windows that contain the full pattern as a subsequence)
	 */
	template <class _report>
	size_t query_y_windows(size_t windowlength, _report * rpt) {
		using namespace std;

		size_t count = 0;
		size_t score = min((size_t)m,windowlength);
		int invalid = -1;
		std::vector<int> inverse_seaweedpermutation(m+n, invalid);
		int j;

		// create inverse permutation
		for (j = 0; j < m+n; ++j) {
			if (seaweedpermutation[j] >= 0) {
				inverse_seaweedpermutation[seaweedpermutation[j]] = j;
			}
		}

		j = -(int)m;
		while(j <= n-windowlength) {
#ifdef _SEAWEEDS_VERIFY
			if(j >= 0) {
				string tmp_text;
				lcs::Llcs<string> _lcs;
				size_t real_lcsl;
				size_t sm_lcsl = windowlength - distribution(j, j+(int)windowlength);
				size_t scm_lcsl;
				tmp_text = y.substr(j, windowlength);
				real_lcsl = _lcs(x, tmp_text);

//				ScoreMatrix<int, ImplicitStorage<Seaweeds<16,16> > > sm(x.size(), tmp_text.size());
//				sm.semilocallcs(x, tmp_text);
//				scm_lcsl = sm.score(0, tmp_text.size());
				scm_lcsl = real_lcsl;

				bool mismatch = false;
				if(real_lcsl != score) {
					mismatch = true;
				}

				if (score != sm_lcsl) {
					mismatch = true;
				}
				if (scm_lcsl != real_lcsl) {
					mismatch = true;
				}
				if(sm_lcsl != real_lcsl)
				if(mismatch) {
					cout << " " << j << " MISMATCH substr = " << tmp_text << endl;
					cout << " " << j << " MISMATCH x = " << x << endl;
					cout << "j\t\t";
					for (int k = 0; k < m+n; ++k) {
						cout << (signed)k-m << "\t";
					}
					cout << endl;
					cout << "k\t\t";
					for (int k = 0; k < m+n; ++k) {
						cout << (signed)k << "\t";
					}
					cout << endl;
					cout << "E[j]\t";
					for (int k = 0; k < m+n; ++k) {
						cout << seaweedpermutation[k] << "\t";
					}
					cout << endl;
					cout << "S[k]\t";
					for (int k = 0; k < m+n; ++k) {
						cout << inverse_seaweedpermutation[k] - m << "\t";
					}
					cout << endl;
					cout << " " << j << " MISMATCH (real) " << real_lcsl << " vs (query) " << sm_lcsl << " vs (sw) "<< score << " vs (scm) " << scm_lcsl << endl;
					throw int(0);
				}
			}
#endif
			if(j >= 0 && score == m) {
				++count;
			}
			if(rpt != NULL && j >= 0) {
				(*rpt)((size_t)j, (double)score);
			}

			if(seaweedpermutation[j+m] >= 0 && seaweedpermutation[j+m] <= j+windowlength) {
				score+= 1;
			}
			if(inverse_seaweedpermutation[j+windowlength] - m >= j) {
				score-= 1;
			}
			++j;
		}

		return count;
	}

protected:
	string x, y;	///< the input strings that resulted in this highest-score matrix
	int m, n; ///< The dimensions of the core.
	boost::shared_ptr<_rangetree> rangetree; ///< pointer to range tree. this will be built the first time the distribution function is called.

	std::vector<int> seaweedpermutation; ///< the seaweed permutation. entry i gives the column for the nonzero in row i-m

	typename _slcsfun::permutation_container right; ///< The seaweed permutation in seaweed distance format, right outputs. Will only be valid directly after call to semilocallcs, kept here to avoid mallocs
	typename _slcsfun::permutation_container top;	///< The seaweed permutation in seaweed distance format, top outputs. Will only be valid directly after call to semilocallcs, kept here to avoid mallocs
private:
	/**
	 * @brief convert seaweed distances to permutation form
	 */
	void seaweed_distances_to_permutation() {
		using namespace std;
		ensure_sizes(m, n);
		for (int j = 0; j < m; ++j) {
			int v = right.get(j);
#ifdef _DEBUG_SEAWEEDS
			cout << "right [" << j << "] = " << v << endl;
#endif // _DEBUG_SEAWEEDS
			if(v < _slcsfun::permutation_container::lsbs) {
				seaweedpermutation[n - v - 1 + m] =(int) (m + n - j - 1);
			} 
#ifdef _DEBUG_SEAWEEDS
			cout << (int)(n - v - 1) << " -> " << (int)(m + n - j - 1) << endl;
#endif // _DEBUG_SEAWEEDS
		}
		for (int j = 0; j < n; ++j) {
			int v = top.get(j);
			if(v < _slcsfun::permutation_container::lsbs) {
				seaweedpermutation[j - v + m] = j;
			}
#ifdef _DEBUG_SEAWEEDS
			cout << (int)(j - v) << " -> " << (int)j << endl;
#endif // _DEBUG_SEAWEEDS
		}
#ifdef _DEBUG_SEAWEEDS
		cout << endl;
#endif // _DEBUG_SEAWEEDS
	}

	/**
	 * @brief make sure the seaweed permutation and distance arrays have the correct sizes
	 */
	void ensure_sizes(int _new_m, int _new_n) {
		int init = -1;
		if(seaweedpermutation.size() < _new_m + _new_n) {
			seaweedpermutation.resize(_new_m + _new_n, init);
		}
		if (right.size() < _new_m) {
			right.resize(_new_m);
		}		
		if (top.size() < _new_n) {
			top.resize(_new_n);
		}		
	}
};

};

#endif
