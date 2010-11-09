/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __NWALIGNMENT_H__
#define __NWALIGNMENT_H__

#include <string>
#include <algorithm>

#include "dynamicprogramming/DynamicProgrammingMatrixSolver.h"
#include "xasmlib/IntegerVector.h"

namespace alignment {

template <class _string = utilities::IntegerVector<8> > 
class PairwiseScoringOperator {
public:
	int subst_matrix[6][6];
	
	double gap_penalty_h;
	double gap_penalty_v;
	double normalization;

private:
	int gap_score_h;
	int gap_score_v;
public:
	PairwiseScoringOperator() {
		for (int i = 0; i < 6; ++i)	{
			for (int j = 0; j < 6; ++j)	{
				if (i == j && i < 4) {
					subst_matrix[i][j] = 2;
				} else {
					subst_matrix[i][j] = 0;
				}
			}
		}

		gap_penalty_h = -0.5;
		gap_penalty_v = -0.5;

		gap_score_h = -1;
		gap_score_v = -1;

		normalization =  0.5;
	}

	static _string translate_input(std:: string const & str, bool first = true) {
		using namespace std;
		_string translated (str.length());
		
		char mismatch_char = first ? 4 : 5;

		for (size_t i = 0; i < translated.size(); ++i) {
			char current = str[i];
			switch (current) {
			case 'A':
			case 'a':
				translated[i] = 0;
				break;
			case 'G':
			case 'g':
				translated[i] = 1;
				break;
			case 'C':
			case 'c':
				translated[i] = 2;
				break;
			case 'T':
			case 't':
				translated[i] = 3;
				break;
			default:
				if('n' != tolower(current)) {
					cerr << "WARNING: untranslated character '" << current << "' at location " << i << 
							" of the " << (first ? "first" : "second") << 
							" input string. This character will mismatch." << endl;

				}
				translated[i] = mismatch_char;
				break;
			}
		}

		return translated;
	}

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
		return max (tleft + gap_score_h,
			max (	ttop + gap_score_v,
					ttop_left + subst_matrix[left][top]
			));
	}

	double convert_score (int score, size_t m, size_t n) {
		return normalization * ((double)score);
	}

};
	
template <class _string = std::string, class _op = PairwiseScoringOperator < > >
class NWAlignment : public dynamic_programming::DynamicProgrammingMatrixSolver<_string, typename _op::score_t, _op> {
public:
	typedef  typename _op :: score_t score_t;
	typedef  typename _op :: dp_element_t dp_element_t;

	_op scoring;

	score_t operator() (_string left_input, _string top_input) {
		return scoring.convert_score(
			((dynamic_programming::
				DynamicProgrammingMatrixSolver<_string, typename _op :: score_t, _op>)*this)(left_input, top_input),
			left_input.size(),
			top_input.size()
		);
	}
};

};

#endif

