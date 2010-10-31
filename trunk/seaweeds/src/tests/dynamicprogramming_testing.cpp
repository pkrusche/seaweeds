/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <algorithm>
#include <vector>

#include "xasmlib/IntegerVector.h"
#include "bspcpp/tools/utilities.h"
#include "dynamicprogramming/dp_matrix.h"

#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"

using namespace std;
using namespace utilities;
using namespace dynamic_programming;

class dp_lcs_op {
public:
	inline void operator() (size_t i, size_t j,
		UINT64 top, 
		UINT64 left,
		int tleft,
		int ttop_left,
		int ttop, 
		int  & tcurrent
	) {
		int character_score = ((char)top) == ((char)left) ? 1 : 0;
		tcurrent = max (tleft, max(ttop, ttop_left + character_score));
//		cout << "D(" << i << ", " << j << ") = " << tcurrent << "\t (c_i = " << left << ", c_j = " << top << ")" << endl;
	}
};

int main(int argc, const char * argv[]) {
	init_xasmlib();
	int testsize = 10000;

	IntegerVector<8> s1(testsize), s2(testsize);
	for (size_t j = 0; j < testsize; ++j) {
		s1[j] = rand() % 4;
		s2[j] = rand() % 4;
	}

	dp_matrix_solver<IntegerVector<8>, int, dp_lcs_op> solver;
	
	solver.cur_left_col->resize(s1.size()+1, 0);
	solver.cur_top_row->resize(s2.size()+1, 0);

	lcs::LlcsCIPR<8> _lcs;
	lcs::Llcs< IntegerVector<8> > _lcs_std;


	cout << "Using DP solver. " << endl;
/*
	cout << "\tinput1: " << s1 << endl;
	cout << "\tinput2: " << s2 << endl;
*/

	double t0 = time();
	int solver_score = solver(s1, s2);
	double t1 = time();
	cout << "Time: " << t1 - t0 << endl << endl;

	cout << "Using bit-parallel LCS. " << endl;
/*
	cout << "\tinput1: " << s1 << endl;
	cout << "\tinput2: " << s2 << endl;
*/

	t0 = time();
	int cipr_score = _lcs(s1, s2);
	t1 = time();
	cout << "Time: " << t1 - t0 << endl << endl;
		
	cout << "Using standard LCS. " << endl;
/*
	cout << "\tinput1: " << s1 << endl;
	cout << "\tinput2: " << s2 << endl;
*/
	t0 = time();
	int std_score = _lcs_std(s1, s2);
	t1 = time();
	cout << "Time: " << t1 - t0 << endl << endl;

	cout << solver_score << " " << cipr_score << " " << std_score << endl;

	return 0;
}

