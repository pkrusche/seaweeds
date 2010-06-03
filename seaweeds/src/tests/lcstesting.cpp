/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>

#include "util/TypeList.h"
#include "bspcpp/tools/utilities.h"

#include "../xasmlib/xasmlib.h"
#include "../xasmlib/IntegerVector.h"

//#define LCS_VERIFY

#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"
#include "tuning/Timing.h"

using namespace std;
using namespace utilities;
using namespace lcs;
using namespace tuning;

template <int _bpc>
class LCSTest {
public:
	LCSTest(size_t _patternsize) : patternsize(_patternsize) {
	}

    double operator() (size_t textsize) {
		patternsize = textsize;

		utilities::IntegerVector<_bpc> test1(textsize), test2(patternsize);

		UINT64 bitmask = ((static_cast<UINT64> (1)) << _bpc) - 1;
		for(size_t j = 0; j < test1.size(); ++j) {
			test1[j] = rand() & bitmask;
		}
		for(size_t j = 0; j < test2.size(); ++j) {
			test2[j] = rand() & bitmask;
		}
		
		lcs::Llcs< utilities::IntegerVector<_bpc> > l;
		lcs::LlcsCIPR<_bpc> lc;

		utilities::time();

		double t1 = utilities::time();
		size_t _l1 = l(test1, test2);
		double tmp = utilities::time();
		t1 = tmp - t1;

		double t2 = utilities::time();
		size_t _l2 = lc(test1, test2);
		tmp = utilities::time();
		t2 = tmp - t2;

		if(_l1 != _l2) {
			cerr << "LCS mismatch " << _l1 << " (LCS) != "<< _l2 << " (CIPR) for sizes "
				<< textsize << ", " << patternsize << ", " << _bpc << " bits per char." << endl;
			cerr << "s1 = " << test1 << endl;
			cerr << "s2 = " << test2 << endl;
		}
		cout << textsize << "\t" << t1 << "\t" << t2 << "\t" << t1/t2 << "\t" << _l1 << "\t" << _l2 << endl;

		return 0;
    }
private:
	size_t patternsize;
};

typedef tuning::Timing < LCSTest< 2 >, size_t > LCSTest2;
typedef tuning::Timing < LCSTest< 3 >, size_t > LCSTest3;
typedef tuning::Timing < LCSTest< 5 >, size_t > LCSTest5;
typedef tuning::Timing < LCSTest< 8 >, size_t > LCSTest8;
typedef tuning::Timing < LCSTest< 15 >, size_t > LCSTest15;
typedef tuning::Timing < LCSTest< 16 >, size_t > LCSTest16;

typedef TYPELIST_6(
	LCSTest2,
	LCSTest3,
	LCSTest5,
	LCSTest8,
	LCSTest15,
	LCSTest16
) testlist;

int main(int argc, char *argv[]) {
	int add = 1000;
	int NUM = 20;
	int inc = 1000;
	int patsize = -1;

	init_xasmlib();

	if(argc > 1) {
		add = atoi(argv[1]);
	}

	if(argc > 2) {
		NUM = atoi(argv[2]);
	}

	if(argc > 3) {
		inc = atoi(argv[3]);
	}

	if(argc > 4) {
		patsize = atoi(argv[4]);
	}

	TimingList tdata;
	MultiTiming <
		size_t,
		testlist
	>::getTimings(add, NUM, inc, tdata, patsize);

	return EXIT_SUCCESS;
}

