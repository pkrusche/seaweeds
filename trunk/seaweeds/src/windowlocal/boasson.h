/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

/**
 *  This is an implementation of
 *  	Luc Boasson, Patrick C�gielski, Ir�ne Guessarian, Yuri Matiyasevich: 
 *      Window-accumulated subsequence matching problem is linear. 
 *      Ann. Pure Appl. Logic 113(1-3): 59-80 (2001)
 * 
 *  A few practical adaptations were made, particularly to make better use
 *  of the IntegerVector class, which implements a more efficient
 *  variant of their MPRAM model.
 * 
 *  Notice that the LCS reporting from this algorithm only provides a lower 
 *  bound on the actual value, as the algorithm only considers maximal 
 *  subsequence occurrences. 
 */

#ifndef _BOASSON_H
#define	_BOASSON_H

#include <iostream>

#include "xasmlib/IntegerVector.h"
#include "lcs/LlcsCIPR.h"

#include "report.h"

namespace windowlocal {

template <size_t _bpc, class _reporter = report_nothing<>, size_t _omega = 16> 
class BoassonMPRAMMatcher {
public:
	typedef utilities::IntegerVector<_omega> STATE_TYPE;

	static const UINT64 msb  = ( static_cast<UINT64>(1) << (_omega) );
	static const UINT64 lsbs = ( msb-1 );

   	BoassonMPRAMMatcher(size_t _window, utilities::IntegerVector<_bpc> const & _pattern) 
		: window(_window) {
		using namespace std;
		ASSERT(log2(_window+2) <= _omega);
		const int alphasize = static_cast<UINT64>(1) << _bpc;

#ifdef _VERBOSETEST_WINDOWLCS_BOASSON
		pattern = _pattern;
#endif /* _VERBOSETEST_WINDOWLCS_BOASSON */
		patsize = _pattern.size();

		for(int j = 0; j < alphasize; ++j) {
			patternmapping_M[j].resize(patsize);
			patternmapping_N[j].resize(patsize);
			patternmapping_M[j].zero();
			patternmapping_N[j].one();
		}
		for(size_t j = 1; j < patsize; ++j) {
			patternmapping_M[(size_t)_pattern[j]][j-1] = lsbs;
		}
		for(size_t j = 0; j < patsize; ++j) {
			patternmapping_N[(size_t)_pattern[j]][j] = 0;
		}


		p1 = _pattern[0];
	}

	size_t operator()(utilities::IntegerVector<_bpc> const & text, _reporter * rpt = NULL, size_t stepsize = 1) {
		using namespace std;
		size_t count = 0;

		STATE_TYPE L(patsize);
		L.zero();
		L.set_bits(lsbs);

		STATE_TYPE T(patsize);
		UINT64 alpha;
		UINT64 delta = lsbs-1-window;
		size_t n = text.size();
		size_t lcslen = 0;

#ifdef _VERBOSETEST_WINDOWLCS_BOASSON
		cout << "t = " << text << endl;
		cout << "p = " << pattern << endl;
#endif /* _VERBOSETEST_WINDOWLCS_BOASSON */

		for(size_t pos = 0; pos < n; ++pos) {
			alpha = text[pos];

			// Get mapping and inverse mapping
			STATE_TYPE & Ma(patternmapping_M[(size_t)alpha]);
			STATE_TYPE & Na(patternmapping_N[(size_t)alpha]);
			
#ifdef _VERBOSETEST2
			cout << pos << " " << alpha << " Ma = " << Ma << endl;
			cout << pos << " " << alpha << " Na = " << Na << endl;
#endif
			// T' = ( (L & Ma) << _omega ) + (L & Na) .+ 1
			T = L;
#ifdef _VERBOSETEST3
			cout << pos << "T = L T = "<< T << endl; 
#endif
			L &= Na;
#ifdef _VERBOSETEST3
			cout << pos << "L & Na L = "<< L << endl; 
#endif // _VERBOSETEST3
			T &= Ma;
#ifdef _VERBOSETEST3
			cout << pos << "T & Ma T = "<< T << endl; 
#endif // _VERBOSETEST3
			T <<= _omega;
#ifdef _VERBOSETEST3
			cout << pos << "T << _omega T = "<< T << endl; 
#endif // _VERBOSETEST3
			L |= T; 
#ifdef _VERBOSETEST3
			cout << pos << "...L or T L = "<< L << endl; 
#endif // _VERBOSETEST3

			// L now contains T'
			if(alpha == p1) {
				L[0]= delta;
#ifdef _VERBOSETEST3
				cout << pos << "...fixl0 L = "<< L << endl; 
#endif // _VERBOSETEST3
			}

			// This operation is emulated by variable E1 in the paper
			L.saturated_inc();
#ifdef _VERBOSETEST3
			cout << pos << "...saturatedinc L = "<< L << endl; 
#endif // _VERBOSETEST3
			
			lcslen = min(patsize, lcslen+1);
			// L[l] gives distance to start of subsequence ending with P[l] or infinity if none
			// also, L[j] grows monotonically with l (if there is a subsequence up to l in the window, 
			// there must also be one up to l-1
			while (lcslen > 0 
				&& L[lcslen-1] == lsbs) {
				--lcslen;
			} 

#ifdef _VERBOSETEST_WINDOWLCS_BOASSON
			size_t ewl = (pos >= window) ? window : pos+1;
			cout << "window " << pos+1-ewl<< "..." << pos  << ":  LCS =  " << lcslen << endl;
			cout << "\tt_w = " << text.substr(pos+1-ewl, ewl) << endl;
			cout << "\tp = " << pattern << endl;
#endif // _VERBOSETEST_WINDOWLCS_BOASSON
#ifndef _NO_MMX
			utilities::do_emms();
#endif
			// report LCS scores
			if(pos >= window-1 && ( (pos-window-1) % stepsize == 0) ) {
				if(rpt != NULL)
					(*rpt)(pos+1-window, lcslen);

				// count subsequence matches
				if( lcslen == patsize ) { 
#ifdef _VERBOSETEST2
					cout << pos << " match!" << endl; 
#endif
					++count;
				}
			}
#ifdef _VERBOSETEST3
			cout << endl;
#endif // _VERBOSETEST3
		}
		return count;
    }

private:
	size_t window;
	size_t patsize;
	STATE_TYPE patternmapping_M[(static_cast<UINT64>(1) << _bpc)+1];
	STATE_TYPE patternmapping_N[(static_cast<UINT64>(1) << _bpc)+1];

#ifdef _VERBOSETEST_WINDOWLCS_BOASSON
	utilities::IntegerVector<_bpc> pattern;
#endif /* _VERBOSETEST_WINDOWLCS_BOASSON */

	UINT64 p1;
};

};

#endif	/* _BOASSON_H */

