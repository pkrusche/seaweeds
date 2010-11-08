/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __NWALIGNMENT_H__
#define __NWALIGNMENT_H__

#include <string>
#include <algorithm>

#include "../dynamicprogramming/dp_matrix.h"

namespace alignment {

class defaultscoring_op {
public:
	typedef double score_t ;
	typedef int dp_element_t;

	inline int operator() (size_t i, size_t j,
		char top, 
		char left,
		int tleft,
		int ttop_left,
		int ttop
	) {
		if(top == left) {
			return ttop_left + 1;
		} else {
			return std::max(tleft, ttop);
		}
	}

	double convert_score(int result_score, size_t m, size_t n) {
		return result_score/2.0;
	}
};

	
template <class _string = std::string, typename _op = defaultscoring_op>
class NWAlignment : public dp_matrix_solver<std::string, typename _op :: score_t, defaultscoring_op> {
	typedef  typename _op :: score_t score_t;
	typedef  typename _op :: dp_element_t dp_element_t;

	score_t operator() (input_t left_input, input_t top_input) {

	}
};

};

#endif

