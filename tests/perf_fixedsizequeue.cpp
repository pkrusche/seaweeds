/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

#include <algorithm>
#include <queue>

#include <bsp_cpp/bsp_cpp.h>
#include "util/FixedSizeQueue.h"

using namespace utilities;

class TestVal : public bsp::ByteSerializable {
public:

	size_t serialized_size() {
		return sizeof(double) + 2*sizeof(int);
	}

	void serialize(void * target, size_t nbytes) {
		char * tc = (char*) target;
		*((double*)tc) = score;
		tc+= sizeof(double);
		*((int*)tc) = dummy1;
		tc+= sizeof(int);
		*((int*)tc) = dummy2;
	}

	void deserialize(void * source, size_t nbytes) {
		char * tc = (char*) source;
		score = *((double*)tc);
		tc+= sizeof(double);
		dummy1 = *((int*)tc);
		tc+= sizeof(int);
		dummy2 = *((int*)tc);
	}

	bool operator< (TestVal const & rhs) const {
		return score < rhs.score;
	}

	double score;
	int dummy1, dummy2;
};

struct tv_less {
	bool operator() (const TestVal & l, const TestVal & r) {
		return l.score >= r.score;
	}
};

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace utilities;

	int size    = 3000;
	int NUM     = 1000000;

	if(argc > 1) {
		size = atoi(argv[1]);
	}

	if(argc > 2) {
		NUM = atoi(argv[2]);
	}

	bsp_warmup(2);
	FixedSizeQueue<TestVal> bq(size);

	srand(1);
	double t0 = bsp_time();
	for(int k = 0; k < NUM; ++k) {
		int ikey = rand();
		double key = ((double)ikey) / (double)RAND_MAX;
		TestVal v;
		v.score = key;
		bq.enqueue(key, v);
	}
	double t1 = bsp_time();

	cout << "[FixedSizeQueue] Queueing of " << NUM << " windows / max " 
		 << size << " windows took " << (t1-t0) << "s" << endl;

	std::priority_queue<TestVal, std::vector<TestVal>, tv_less> pq;

	srand(1);
	t0 = bsp_time();
	for(int k = 0; k < NUM; ++k) {
		int ikey = rand();
		double key = ((double)ikey) / (double)RAND_MAX;
		TestVal v;
		v.score = key;

		if(pq.size() >= size && key < pq.top().score) {
			continue;
		}
		pq.push(v);
		if(pq.size() > size) {
			pq.pop();
		}
	} 
	t1 = bsp_time();

	std::vector<TestVal> v1;
	std::vector<TestVal> v2;

	bq.get_all(v1);
	while (!pq.empty()) {
		v2.push_back(pq.top());
		pq.pop();
	}

	std::sort(v1.begin(), v1.end(), tv_less());
	std::sort(v2.begin(), v2.end(), tv_less());

	if(v1.size() != v2.size()) {
		cerr << "Size mismatch!" << endl;
	} else {
		for (int i = 0; i < v1.size(); ++i) {
			if(fabs(v1[i].score - v2[i].score) > 0.0001) {
				cerr << "Value mismatch: " << i << endl;				
			}
		}
	}

	cout << "[std::priority_queue] Queueing of " << NUM << " windows / max " 
		 << size << " windows took " << (t1-t0) << "s" << endl;


	return 0;
}

