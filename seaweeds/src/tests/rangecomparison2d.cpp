/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include <iostream>
#include <math.h>
#include <list>
#include <vector>
#include <map>

#include "../autoconfig.h"

#include "xasmlib/functors.h"
#include "rangesearching/RangeBenchmark2D.h"
#include "rangesearching/Range2D.h"

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace rangesearching;

	int add = 0;
	int NUM = 128;
	int inc = 4;

	if(argc > 1) {
		add = atoi(argv[1]);
	}

	if(argc > 2) {
		NUM = atoi(argv[2]);
	}

	if(argc > 3) {
		inc = atoi(argv[3]);
	}

	typedef pair<const char*, vector<double> > timing;
	list<timing> timings;
	utilities::warmup();
	{
		timing t;
		t.first = "2DLL";
		t.second.resize(NUM);
		RangeBenchmark2D<Point2D<int>, size_t, Range2DLL<int> > benchmark;

		for(int num = 0; num < NUM; ++num) {
			size_t actual = num*inc + add;
			t.second[num] = benchmark(actual, 0);
		}
		timings.insert(timings.end(), t);
	}
	{
		timing t;
		t.first = "2DTL";
		t.second.resize(NUM);
		RangeBenchmark2D<Point2D<int>, size_t, Range2DTL<int> > benchmark;

		for(int num = 0; num < NUM; ++num) {
			size_t actual = num*inc + add;
			t.second[num] = benchmark(actual, 0);
		}
		timings.insert(timings.end(), t);
	}
	{
		timing t;
		t.first = "2DTT";
		t.second.resize(NUM);
		RangeBenchmark2D<Point2D<int>, size_t, Range2DTT<int> > benchmark;

		for(int num = 0; num < NUM; ++num) {
			size_t actual = num*inc + add;
			t.second[num] = benchmark(actual, 0);
		}
		timings.insert(timings.end(), t);
	}

	cout << "n\t";
	for(list<timing>::iterator it = timings.begin(); it != timings.end(); ++it) {
		cout << it->first << "\t";
	}
	cout << endl;

	for(int num = 0; num < NUM; ++num) {
		size_t actual = num*inc + add;
		cout << actual << "\t";
		for(list<timing>::iterator it = timings.begin(); it != timings.end(); ++it) {
			cout << it->second[num] << "\t";
		}
		cout << endl;
	}

	return EXIT_SUCCESS;
}
