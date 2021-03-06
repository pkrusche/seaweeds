/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

#include "bsp.h"
#include "xasmlib/IntegerVector.h"
#include "dynamicprogramming/DynamicProgrammingMatrixSolver.h"

#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"

using namespace std;
using namespace utilities;
using namespace dynamic_programming;

class dp_lcs_op {
public:
	inline int operator() (size_t i, size_t j,
		UINT64 top, 
		UINT64 left,
		int tleft,
		int ttop_left,
		int ttop
	) {
/*
#ifdef _WIN32
		char character_score;

		__asm mov dl, left
		__asm mov cl, top
		__asm cmp cl, dl
		__asm sete character_score
#else
		int character_score = ((char)top) == ((char)left) ? 1 : 0;
#endif

		return max (tleft, max(ttop, ttop_left + character_score));
*/
		if(top == left) {
			return ttop_left + 1;
		} else {
			return max(tleft, ttop);
		}

//		cout << "D(" << i << ", " << j << ") = " << tcurrent << "\t (c_i = " << left << ", c_j = " << top << ")" << endl;
	}

	inline int init_l() {
		return 0;
	}

	inline int init_t() {
		return 0;
	}
};

int main(int argc, const char * argv[]) {
	init_xasmlib();
	int testsize = 10000;

	IntegerVector<8> s1, s2;
	s1.resize(testsize);
	s2.resize(testsize);
	for (size_t j = 0; j < testsize; ++j) {
		s1[j] = rand() % 256;
		s2[j] = rand() % 256;
	}

	DynamicProgrammingMatrixSolver<IntegerVector<8>, int, dp_lcs_op> solver;
	
	solver.cur_left_col->resize(testsize+1, 0);
	solver.cur_top_row->resize(testsize+1, 0);

	lcs::Llcs< IntegerVector<8> > _lcs_std;

	cout << "Using DP solver. " << endl;

	double t0 = bsp_time();
	int solver_score = solver(s1, s2);
	double t1 = bsp_time();
	cout << "Time: " << t1 - t0 << endl << endl;

	cout << "Using standard LCS. " << endl;
	t0 = bsp_time();
	int std_score = (int)_lcs_std(s1, s2);
	t1 = bsp_time();
	cout << "Time: " << t1 - t0 << endl << endl;

	cout << solver_score << " " << std_score << endl;

	return 0;
}

