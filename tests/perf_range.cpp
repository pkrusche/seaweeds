/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <math.h>
#include <list>
#include <vector>
#include <map>

#include "bsp.h"

#include "xasmlib/functors.h"
#include "rangesearching/RangeBenchmark.h"
#include "rangesearching/RangeList.h"
#include "rangesearching/BinTree.h"
#include "rangesearching/RangeTree.h"

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

	bsp_warmup(2);
	{
		timing t;
		t.first = "lsl, nc";
		t.second.resize(NUM);
		RangeBenchmark<int, size_t, RangeList<int, std::less<int>, functors::count<int>, false> > benchmark;

		for(int num = 0; num < NUM; ++num) {
			size_t actual = num*inc + add;
			t.second[num] = benchmark(actual, 0);
		}
		timings.insert(timings.end(), t);
	}
	{
		timing t;
		t.first = "lsl, c";
		t.second.resize(NUM);
		RangeBenchmark<int, size_t, RangeList<int, std::less<int>, functors::count<int>, true> > benchmark;

		for(int num = 0; num < NUM; ++num) {
			size_t actual = num*inc + add;
			t.second[num] = benchmark(actual, 0);
		}
		timings.insert(timings.end(), t);
	}
	{
		timing t;
		t.first = "bt, nc";
		t.second.resize(NUM);
		RangeBenchmark<int, size_t, BinTree<int, std::less<int>, functors::count<int>, false> > benchmark;

		for(int num = 0; num < NUM; ++num) {
			size_t actual = num*inc + add;
			t.second[num] = benchmark(actual, 0);
		}
		timings.insert(timings.end(), t);
	}
	{
		timing t;
		t.first = "bt, c";
		t.second.resize(NUM);
		RangeBenchmark<int, size_t, BinTree<int, std::less<int>, functors::count<int>, true> > benchmark;

		for(int num = 0; num < NUM; ++num) {
			size_t actual = num*inc + add;
			t.second[num] = benchmark(actual, 0);
		}
		timings.insert(timings.end(), t);
	}
	{
		timing t;
		t.first = "rt";
		t.second.resize(NUM);
		RangeBenchmark<int, size_t, RangeTree<int, std::less<int>, functors::count<int> > > benchmark;

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
