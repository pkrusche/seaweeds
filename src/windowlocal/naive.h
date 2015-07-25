/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __WL_NAIVE_H__
#define __WL_NAIVE_H__

#include "xasmlib/IntegerVector.h"
#include "report.h"

namespace windowlocal {

template < class _llcs > 
class WindowLocalLCS {
public:

	// llcs must provide the string class we will use
	typedef typename _llcs::string string;

	WindowLocalLCS(int _windowlength, string const & _pattern) 
		: windowlength(_windowlength), pattern(_pattern) {}

	/** count matches of pattern in text, report windowlength-lcs lengths */
	int count(string const & text, 
		window_reporter * rpt = NULL, 
		int text_p0 = 0,
		int pat_p0 = 0
		) {
		using namespace std;

		static _llcs llcs;

		ASSERT((int)text.size() >= windowlength);

		int n = (int)text.size() - windowlength+1, 
			p = (int)pattern.size();
		
		int count = 0;
		

#ifdef _VERBOSETEST_WINDOWlengthLCS
		cout << "t = " << text << endl;
		cout << "p = " << pattern << endl;
#endif /* _VERBOSETEST_WINDOWlengthLCS */
		
		for(int j = 0; j < n; j++) {
			string current_windowlength(text.substr(j, windowlength));
			double lcslen = (double)llcs(pattern, current_windowlength);

#ifdef _VERBOSETEST_WINDOWlengthLCS
			cout << "windowlength " << j << "..." << j+windowlength  << ":  LCS =  " << lcslen << endl;
			cout << "\tt_w = " << current_windowlength << endl;
#endif // _VERBOSETEST_WINDOWlengthLCS

			if(rpt != NULL) {
				rpt->report_score(window (j+text_p0, pat_p0, lcslen));
			}

			if(lcslen > p-1) {
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
		windowlength = _windowlength;
	}

private:
	int windowlength;
	string const & pattern;
};

};

#endif
