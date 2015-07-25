/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>

#include "util/TypeList.h"

#include "UnitTest++.h"

#include "xasmlib/xasmlib.h"
#include "xasmlib/IntegerVector.h"

#define LCS_VERIFY

#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"
#include "tuning/Timing.h"

using namespace UnitTest;
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

		size_t _l1 = l(test1, test2);
		size_t _l2 = lc(test1, test2);

		CHECK_EQUAL(_l1, _l2);

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

typedef TYPELIST_4(
	LCSTest2,
	LCSTest3,
	LCSTest5,
	LCSTest8
//	LCSTest15,	// these take bloody ages.
//	LCSTest16
) testlist;

namespace {
	SUITE(Lengthy) {
	TEST(Test_LCS_Result)
	{
		int add = 100;
		int NUM = 5;
		int inc = 7;
		int patsize = -1;

		init_xasmlib();
		TimingList tdata;
		MultiTiming <
			size_t,
			testlist
		>::getTimings(add, NUM, inc, tdata, patsize);
	}
	}
};


