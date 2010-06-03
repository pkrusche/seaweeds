/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __WINDOWLOCAL_SCOREMATRIX_H__
#define __WINDOWLOCAL_SCOREMATRIX_H__

#include "util/TypeList.h"
#include "lcs/Llcs.h"
#include "xasmlib/IntegerVector.h"

namespace windowlocal {

template <class _reporter, class scorematrix>
class ScoreMatrixWindowLocalLlcs {
public:
	typedef typename scorematrix::string string;

	ScoreMatrixWindowLocalLlcs(size_t _window, typename scorematrix::string const & _pattern)
		: window(_window) {
		pattern = _pattern;
	}

	size_t operator()(
		string const & text,
		_reporter * rpt = NULL, size_t stepsize = 1
	) {
		using namespace std;
		using namespace utilities;

		ASSERT(text.size() >= window);

		scorematrix m(pattern.size(), text.size());
		m.semilocallcs(pattern, text);
		return m.query_y_windows(window, stepsize, rpt);
	}

private:
	size_t window;
	string pattern;
};

};
#endif
