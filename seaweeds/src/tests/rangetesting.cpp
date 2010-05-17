/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include "config.h"

#include <iostream>
#include <math.h>

#include "xasmlib/functors.h"
#include "rangesearching/RangeTest.h"
#include "rangesearching/RangeTest2D.h"

#include "rangesearching/RangeList.h"
#include "rangesearching/BinTree.h"
#include "rangesearching/RangeTree.h"
#include "rangesearching/Range2D.h"

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace rangesearching;

	int add = 0;
	int NUM = 512;
	int inc = 4;
	size_t size = RAND_MAX;

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
		size = atoi(argv[4]);
	}

	int fails = 0;

	cout << "Testing RangeList range" << endl;
	for(int num = 0; num < NUM; ++num) {
		int l = 0;		
		{ 
			RangeTest<int, size_t, RangeList<int, std::less<int>, functors::count<int>, false> > test;
			if(!test(num*inc + add, 0)) {
				++fails;
				cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
			} 
		}
		{ 
			RangeTest<int, size_t, RangeList<int, std::less<int>, functors::count<int>, true> > test;
			if(!test(num*inc + add, 0)) {
				++fails;
				cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
			} 
		}
	}

	cout << "Testing BinTree range" << endl;
	for(int num = 0; num < NUM; ++num) {
		int l = 0;		
		{ 
			RangeTest<int, size_t, BinTree<int, std::less<int>, functors::count<int>, false> > test;
			if(!test(num*inc + add, 0)) {
				++fails;
				cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
			} 
		}
		{ 
			RangeTest<int, size_t, BinTree<int, std::less<int>, functors::count<int>, true> > test;
			if(!test(num*inc + add, 0)) {
				++fails;
				cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
			} 
		}
	}

	cout << "Testing RangeTree range" << endl;
	for(int num = 0; num < NUM; ++num) {
		int l = 0;		
		RangeTest<int, size_t, RangeTree<int, std::less<int>, functors::count<int> > > test;
		if(!test(num*inc + add, 0)) {
			++fails;
			cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
		} 

	}

	cout << "Testing 2D range LL" << endl;
	for(int num = 0; num < NUM; ++num) {
		int l = 0;		
		RangeTest2D<Point2D<int>, size_t, Range2DLL<int> > test;
		if(!test(num*inc + add, 0)) {
			++fails;
			cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
		} 
	}

	cout << "Testing 2D range TL" << endl;
	for(int num = 0; num < NUM; ++num) {
		int l = 0;		
		RangeTest2D<Point2D<int>, size_t, Range2DTL<int> > test;
		if(!test(num*inc + add, 0)) {
			++fails;
			cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
		} 
	}

	cout << "Testing 2D range TT" << endl;
	for(int num = 0; num < NUM; ++num) {
		int l = 0;		
		RangeTest2D<Point2D<int>, rs_container<Point2D<int> >, Range2DTT<int, functors::report<Point2D<int> > > > test;
		rs_container<Point2D<int> > v; 
		if(!test(num*inc + add, v)) {
			++fails;
			cout << "Failed test at " << __FILE__ << ":" << __LINE__ << endl;
		} 
	}

	if (fails == 0) {
		cout << "OK." << endl;
	} else {
		cout << "There were " << fails << " failed tests." << endl;
	}

	return EXIT_SUCCESS;
}
