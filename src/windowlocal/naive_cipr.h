/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __WL_NAIVE_CIPR_H__
#define __WL_NAIVE_CIPR_H__

#include "xasmlib/IntegerVector.h"
#include "lcs/LlcsCIPR.h"
#include "report.h"

namespace windowlocal {

template < int _bpc > 
class BPWindowLocalLCS {
public:
	typedef utilities::IntegerVector<_bpc> string; 

	BPWindowLocalLCS(int _window, string const & _pattern) 
		: window(_window), 
		  pattern(_pattern) {
	}

	/** count matches of pattern in text, report windowlength-lcs lengths */
	int count(string const & text, 
		window_reporter * rpt = NULL, 
		int text_p0 = 0,
		int pat_p0 = 0
		) {
		ASSERT(text.size() >= window);
		int n = (int)(text.size() - window + 1), 
		 	p = pattern.size();
		int count = 0;
		static lcs::LlcsCIPR<_bpc> llcs;

		utilities::CharMapping<_bpc, 1, false> 
			pattern_mapping(pattern);

#ifdef _VERBOSETEST_WINDOWLCS_CIPR
		cout << "t = " << text << endl;
		cout << "p = " << pattern << endl;
#endif /* _VERBOSETEST_WINDOWLCS_CIPR */
		string current_window(window);
		for(int j = 0; j < n; j+= 1) {
			text.extract_substring(j, j+window-1, current_window);
			int lcslen = llcs(p, pattern_mapping, current_window);

#ifdef _VERBOSETEST_WINDOWLCS_CIPR
			cout << "window " << j << "..." << j+window  << ":  LCS =  " << lcslen << endl;
			cout << "\tt_w = " << current_window << endl;
#endif // _VERBOSETEST_WINDOWLCS_CIPR

			if(rpt != NULL) {
				rpt->report_score(windowlocal::window(j+text_p0, pat_p0, (double)lcslen));
			}

			if(lcslen == p) {
				++count;
			}
		}

		return count;
	}

	/** set the pattern */
	void set_pattern(string _pattern) {
		pattern = _pattern;
	}

	/* set the window length */ 
	void set_windowlength(int _windowlength) {
		window = _windowlength;
	}


private:
	string pattern;
	int window;
};

};

#endif
