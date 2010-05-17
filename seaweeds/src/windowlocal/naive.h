/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __WL_NAIVE_H__
#define __WL_NAIVE_H__

#include "xasmlib/IntegerVector.h"
#include "report.h"

namespace windowlocal {

template <typename _llcs, class string, class _reporter = report_nothing<> > 
class NaiveWindowLocalLlcs {
public:
	NaiveWindowLocalLlcs(size_t _window, string const & _pattern) 
		: window(_window), pattern(_pattern) {}
	
	size_t operator()(string const & text, _reporter * rpt = NULL, size_t stepsize = 1) {
		using namespace std;
		ASSERT(text.size() >= window);
		size_t n = text.size() - window+1, p = pattern.size();
		size_t count = 0;
		_llcs llcs;

#ifdef _VERBOSETEST_WINDOWLCS
		cout << "t = " << text << endl;
		cout << "p = " << pattern << endl;
#endif /* _VERBOSETEST_WINDOWLCS */
		
		for(size_t j = 0; j < n; j+= stepsize) {
			string current_window(text.substr(j, window));
			size_t lcslen = llcs(pattern, current_window);

#ifdef _VERBOSETEST_WINDOWLCS
			cout << "window " << j << "..." << j+window  << ":  LCS =  " << lcslen << endl;
			cout << "\tt_w = " << current_window << endl;
#endif // _VERBOSETEST_WINDOWLCS

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
	size_t window;
	string const & pattern;
};

};

#endif
