/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __WL_NAIVE_CIPR_H__
#define __WL_NAIVE_CIPR_H__

#include "xasmlib/IntegerVector.h"
#include "lcs/LlcsCIPR.h"
#include "report.h"

namespace windowlocal {

template <size_t _bpc, class _reporter = report_nothing<> > 
class NaiveBitparallelWindowLocalLlcs {
public:
	NaiveBitparallelWindowLocalLlcs(size_t _window, utilities::IntegerVector<_bpc> const & _pattern) 
		: window(_window), pattern_mapping(_pattern), patlen(_pattern.size()) {

#ifdef _VERBOSETEST_WINDOWLCS_CIPR
		pattern = _pattern;
#endif /* _VERBOSETEST_WINDOWLCS_CIPR */
	}
	
	size_t operator()(utilities::IntegerVector<_bpc> const & text, _reporter * rpt = NULL, size_t stepsize = 1) {
		ASSERT(text.size() >= window);
		size_t n = text.size() - window + 1, p = patlen;
		size_t count = 0;
		lcs::LlcsCIPR<_bpc> llcs;

#ifdef _VERBOSETEST_WINDOWLCS_CIPR
		cout << "t = " << text << endl;
		cout << "p = " << pattern << endl;
#endif /* _VERBOSETEST_WINDOWLCS_CIPR */
		utilities::IntegerVector<_bpc> current_window(window);
		for(size_t j = 0; j < n; j+= stepsize) {
			text.extract_substring(j, j+window-1, current_window);
			size_t lcslen = llcs(patlen, pattern_mapping, current_window);

#ifdef _VERBOSETEST_WINDOWLCS_CIPR
			cout << "window " << j << "..." << j+window  << ":  LCS =  " << lcslen << endl;
			cout << "\tt_w = " << current_window << endl;
#endif // _VERBOSETEST_WINDOWLCS_CIPR

			if(rpt != NULL) {
				(*rpt) (j, (double)lcslen);
			}

			if(lcslen == p) {
				++count;
			}
		}

		return count;
	}

private:
#ifdef _VERBOSETEST_WINDOWLCS_CIPR
	utilities::IntegerVector<_bpc> pattern;
#endif /* _VERBOSETEST_WINDOWLCS_CIPR */

	size_t window;
	utilities::CharMapping<_bpc, 1, false> pattern_mapping;
	size_t patlen;
};

};

#endif
