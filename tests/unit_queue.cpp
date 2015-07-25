/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>

#include "xasmlib/Queue.h"
#include "util/Permutation.h"

#include "UnitTest++.h"
using namespace UnitTest;

using namespace std;
using namespace utilities;

namespace {
	TEST(Test_Queue)
	{
		Queue<UINT64> q;
		std::vector<UINT64> v;
		v.resize(100);

		// push 100 values with duplicates in random order, see if they come out sorted
		for (int i = 0; i < 100; ++i) {
			v[i] = i/2;
		}

		for (int k = 0; k < 1000; ++k) {
			CHECK_EQUAL(0, q.size());

			Permutation< std::vector<UINT64> > p(v);
			p.randomize();
			for (int i = 0; i < 100; ++i) {
				q.push(p[i]);
			}

			for (int i = 0; i < 50; ++i) {
				// cout << i << "\t" << p[i] << "\t" << q.top() << endl;
				CHECK(q.top() <= 50-i);
				CHECK_EQUAL(100-2*i, q.size());
				q.pop(50-i-1);
			}
		}

	}
}
