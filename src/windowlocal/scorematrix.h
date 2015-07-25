/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __WINDOWLOCAL_SCOREMATRIX_H__
#define __WINDOWLOCAL_SCOREMATRIX_H__

#include "util/TypeList.h"
#include "lcs/Llcs.h"
#include "xasmlib/IntegerVector.h"

#include "report.h"

namespace windowlocal {

struct Report_1D {
	void operator() (int pos, double score) {
		if(rpt) {
			rpt->report_score(window(text_p0 + pos, pat_p0, score));
		}
	}
	window_reporter * rpt;
	int text_p0;
	int pat_p0;
} _rpt;


template <class scorematrix>
class ScoreMatrixWindowLocalLCS {
public:
	typedef typename scorematrix::string string;

	ScoreMatrixWindowLocalLCS(size_t _window, typename scorematrix::string const & _pattern)
		: window(_window) {
		pattern = _pattern;
	}

	int count(string const & text, 
		window_reporter * rpt = NULL, 
		int text_p0 = 0,
		int pat_p0 = 0
	) {
		using namespace std;
		using namespace utilities;

		ASSERT(text.size() >= window);

		scorematrix m((int)pattern.size(), (int)text.size());
		m.semilocallcs(pattern, text);

		Report_1D _rpt;
		_rpt.text_p0 = text_p0;
		_rpt.pat_p0  = pat_p0;
		_rpt.rpt     = rpt;

		return m.query_y_windows(window, &_rpt);
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
	size_t window;
	string pattern;
};

};
#endif
