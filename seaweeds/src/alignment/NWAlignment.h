/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __NWALIGNMENT_H__
#define __NWALIGNMENT_H__

#include <string>
#include <algorithm>

#include "dynamicprogramming/dp_matrix.h"

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
		using namespace std;
		if(top == left) {
			return ttop_left + 1;
		} else {
			return max(tleft, ttop);
		}
	}

	double convert_score(int result_score, size_t m, size_t n) {
		return result_score/2.0;
	}
};

	
template <class _string = std::string, class _op = defaultscoring_op>
class NWAlignment : public dynamic_programming::dp_matrix_solver<_string, typename _op::score_t, _op> {
public:
	typedef  typename _op :: score_t score_t;
	typedef  typename _op :: dp_element_t dp_element_t;

	_op scoring;

	score_t operator() (_string left_input, _string top_input) {
		return scoring.convert_score(
			((dp_matrix_solver<_string, typename _op :: score_t, _op>)*this)(left_input, top_input),
			left_input.size(),
			top_input.size()
		);
	}
};

};

#endif

