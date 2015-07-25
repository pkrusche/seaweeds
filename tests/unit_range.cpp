/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "UnitTest++.h"

#include <iostream>
#include <math.h>

#include "xasmlib/functors.h"
#include "rangesearching/RangeTest.h"
#include "rangesearching/RangeTest2D.h"

#include "rangesearching/RangeList.h"
#include "rangesearching/BinTree.h"
#include "rangesearching/RangeTree.h"
#include "rangesearching/Range2D.h"

using namespace std;
using namespace rangesearching;
using namespace UnitTest;


namespace {
	const int add = 0;
	const int NUM = 128;
	const int inc = 16;
	const size_t size = RAND_MAX;
	
	SUITE(Lengthy) {

	TEST(Test_RangeList) {
		int fails = 0;

		for(int num = 0; num < NUM; ++num) {
			int l = 0;		
			{ 
				RangeTest<int, size_t, RangeList<int, std::less<int>, functors::count<int>, false> > test;
				CHECK(test(num*inc + add, 0));
			}
			{ 
				RangeTest<int, size_t, RangeList<int, std::less<int>, functors::count<int>, true> > test;
				CHECK(test(num*inc + add, 0));
			}
		}
	}

	TEST(Test_BinTree) {
		for(int num = 0; num < NUM; ++num) {
			int l = 0;		
			{ 
				RangeTest<int, size_t, BinTree<int, std::less<int>, functors::count<int>, false> > test;
				CHECK(test(num*inc + add, 0));
			}
			{ 
				RangeTest<int, size_t, BinTree<int, std::less<int>, functors::count<int>, true> > test;
				CHECK(test(num*inc + add, 0));
			}
		}
	}

	TEST(Test_RangeTree) {
		for(int num = 0; num < NUM; ++num) {
			int l = 0;		
			RangeTest<int, size_t, RangeTree<int, std::less<int>, functors::count<int> > > test;
			CHECK (test(num*inc + add, 0));
		}
	}

	TEST(Test_Range_2D_LL) {
		for(int num = 0; num < NUM; ++num) {
			int l = 0;		
			RangeTest2D<Point2D<int>, size_t, Range2DLL<int> > test;
			CHECK(test(num*inc + add, 0));
		}
	}

	TEST(Test_Range_2D_TL) {
		for(int num = 0; num < NUM; ++num) {
			int l = 0;		
			RangeTest2D<Point2D<int>, size_t, Range2DTL<int> > test;
			CHECK(test(num*inc + add, 0));
		}
	}

	TEST(Test_Range_2D_TT) {
		for(int num = 0; num < NUM; ++num) {
			int l = 0;		
			RangeTest2D<Point2D<int>, rs_container<Point2D<int> >, Range2DTT<int, functors::report<Point2D<int> > > > test;
			rs_container<Point2D<int> > v; 
			CHECK(test(num*inc + add, v));
		}
	}
	}
};
