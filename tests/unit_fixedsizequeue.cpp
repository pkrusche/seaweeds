/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

#include <algorithm>

#include "UnitTest++.h"

#include "util/FixedSizeQueue.h"

using namespace UnitTest;
using namespace utilities;

#define TEST_LEN 1000

namespace {

	/** Test class holding a string value and a double key */
	class Value : public bsp::ByteSerializable {
	public:
		double v;
		std::string sv;
		
		Value(double _v = 0, std::string _sv = "") : v(_v), sv(_sv) {
		}


		void serialize(void * target, size_t nbytes) {
			ASSERT(nbytes >= serialized_size());
			char * tc = (char*)target;
			*((double*)tc) = v;
			tc+= sizeof(double);
			*((size_t*)tc) = sv.size();
			tc+= sizeof(size_t);
			memcpy(tc, sv.c_str(), sv.size());
			tc+= sv.size();
			*tc = 0;
		}

		void deserialize(void * source, size_t nbytes) {
			char * tc = (char*)source;
			v = *((double*)tc);
			tc+= sizeof(double);
			size_t sz = *((size_t*)tc);
			tc+= sizeof(size_t);
			sv = std::string(tc, sz);
		}

		size_t serialized_size() {
			return sizeof(double) + sizeof(size_t) + sv.size() + 1;
		}
		
		Value const & operator=(Value const & rhs) {
			v = rhs.v;
			sv = rhs.sv;
			return *this;
		}
	};

	bool operator==(Value const & v1, Value const & v2) {
		return v1.sv == v2.sv && fabs(v1.v-v2.v) < 0.000001;
	}

	struct Value_less {
		bool operator() (Value const & l, Value const & r) {
			return l.v < r.v;
		}
	};

	TEST(Test_FixedSizeQueue_AddElems) {
		FixedSizeQueue<int> q1(100);
		FixedSizeQueue<Value> q2(100);

		std::vector<int> vs;
		vs.resize(TEST_LEN);

		for(int k = 0; k < TEST_LEN; ++k) {
			int ikey = rand();
			double key = ((double)ikey) / (double)RAND_MAX;
			char cc[128];
			sprintf(cc, "ik : %i", ikey);
			std::string skey(cc);

			Value v(key, skey);
			q1.enqueue(key, ikey);
			q2.enqueue(key, v);
			vs[k] = ikey;
			// std::cerr << k << "\t" << ikey << "\t" << key << std::endl;
		}

		std::sort(vs.begin(), vs.end());
		std::vector<int> v2;
		q1.get_all(v2);

		std::vector<Value> v3;
		q2.get_all(v3);

		CHECK_EQUAL(100, v2.size());
		CHECK_EQUAL(100, v3.size());

		std::sort(v2.begin(), v2.end());

		std::sort(v3.begin(), v3.end(), Value_less());

		for(int k = 0; k < 100; ++k) {
			// last 100 elements should be kept
			int ishouldbe = vs[k+vs.size()-100];
			double dshouldbe = ((double)ishouldbe) / (double)RAND_MAX;
			char cc[128];
			sprintf(cc, "ik : %i", ishouldbe);
			std::string sshouldbe(cc);
			
			CHECK_EQUAL(ishouldbe, v2[k]);
			CHECK_CLOSE(dshouldbe, v3[k].v, 0.0000001);
			CHECK_EQUAL(sshouldbe, v3[k].sv);
		}
		int ishouldbe = vs[vs.size()-100];
		double dshouldbe = ((double)ishouldbe) / (double)RAND_MAX;
		CHECK_CLOSE(dshouldbe, q1.get_min_key(), 0.0001);
		CHECK_CLOSE(dshouldbe, q2.get_min_key(), 0.0001);
	}


	TEST(Test_FixedSizeQueue_Assign) {
		FixedSizeQueue<int> q1(100);
		FixedSizeQueue<Value> q2(100);

		for(int k = 0; k < 50; ++k) {
			int ikey = rand();
			double key = ((double)ikey) / (double)RAND_MAX;
			char cc[128];
			sprintf(cc, "ik : %i", ikey);
			std::string skey(cc);

			Value v(key, skey);
			q1.enqueue(key, ikey);
			q2.enqueue(key, v);
			// std::cerr << k << "\t" << ikey << "\t" << key << std::endl;
		}

		FixedSizeQueue<int> q1a = q1;
		FixedSizeQueue<Value> q2a = q2;

		std::vector<int> v2, v2a;
		q1.get_all(v2);
		q1a.get_all(v2a);

		CHECK_EQUAL(v2.size(), v2a.size());

		CHECK_EQUAL(q1.size(), q1a.size());
		CHECK_CLOSE(q1.get_min_key(), q1a.get_min_key(), 0.0001);

		for(size_t s = 0; s < v2.size(); ++s) {
			CHECK_EQUAL(v2[s], v2a[s]);
		}

		std::vector<Value> v3, v3a;
		q2.get_all(v3);
		q2a.get_all(v3a);

		CHECK_EQUAL(v3.size(), v3a.size());
		CHECK_EQUAL(q2.size(), q2a.size());
		CHECK_CLOSE(q2.get_min_key(), q2a.get_min_key(), 0.0001);

		for(size_t s = 0; s < v3.size(); ++s) {
			CHECK(v3[s] == v3a[s]);
		}
	}

	TEST(Test_FixedSizeQueue_Serialize) {
		FixedSizeQueue<int> q1(100);
		FixedSizeQueue<Value> q2(100);

		for(int k = 0; k < 50; ++k) {
			int ikey = rand();
			double key = ((double)ikey) / (double)RAND_MAX;
			char cc[128];
			sprintf(cc, "ik : %i", ikey);
			std::string skey(cc);

			Value v(key, skey);
			q1.enqueue(key, ikey);
			q2.enqueue(key, v);
			// std::cerr << k << "\t" << ikey << "\t" << key << std::endl;
		}

		FixedSizeQueue<int> q1a;
		FixedSizeQueue<Value> q2a;

		size_t sz = q1.serialized_size();
		char * data = new char[sz];
		q1.serialize(data, sz);
		q1a.deserialize(data, sz);
		delete [] data;

		sz = q2.serialized_size();
		char * data2 = new char[sz];
		q2.serialize(data2, sz);
		q2a.deserialize(data2, sz);
		delete [] data2;

		std::vector<int> v2, v2a;
		q1.get_all(v2);
		q1a.get_all(v2a);

		CHECK_EQUAL(v2.size(), v2a.size());

		CHECK_EQUAL(q1.size(), q1a.size());
		CHECK_CLOSE(q1.get_min_key(), q1a.get_min_key(), 0.0001);

		for(size_t s = 0; s < v2.size(); ++s) {
			CHECK_EQUAL(v2[s], v2a[s]);
		}

		std::vector<Value> v3, v3a;
		q2.get_all(v3);
		q2a.get_all(v3a);

		CHECK_EQUAL(v3.size(), v3a.size());
		CHECK_EQUAL(q2.size(), q2a.size());
		CHECK_CLOSE(q2.get_min_key(), q2a.get_min_key(), 0.0001);

		for(size_t s = 0; s < v3.size(); ++s) {
			CHECK(v3[s] == v3a[s]);
		}		
	}

	TEST(Test_FixedSizeQueue_Reduce) {
		FixedSizeQueue<int> q1(100);
		FixedSizeQueue<Value> q2(100);
		FixedSizeQueue<int> q1a(100);
		FixedSizeQueue<Value> q2a(100);

		std::vector<int> vs;
		vs.resize(2000);

		for(int k = 0; k < 1000; ++k) {
			int ikey = rand();
			double key = ((double)ikey) / (double)RAND_MAX;
			char cc[128];
			sprintf(cc, "ik : %i", ikey);
			std::string skey(cc);

			Value v(key, skey);
			q1.enqueue(key, ikey);
			q2.enqueue(key, v);
			vs[k] = ikey;
			// std::cerr << k << "\t" << ikey << "\t" << key << std::endl;
		}

		for(int k = 1000; k < 2000; ++k) {
			int ikey = rand();
			double key = ((double)ikey) / (double)RAND_MAX;
			char cc[128];
			sprintf(cc, "ik : %i", ikey);
			std::string skey(cc);

			Value v(key, skey);
			q1a.enqueue(key, ikey);
			q2a.enqueue(key, v);
			vs[k] = ikey;
			// std::cerr << k << "\t" << ikey << "\t" << key << std::endl;
		}

		std::sort(vs.begin(), vs.end());

		q1.reduce_with(&q1a);
		q2.reduce_with(&q2a);

		std::vector<int> v1;
		q1.get_all(v1);
		std::sort(v1.begin(), v1.end());

		std::vector<Value> v2;
		q2.get_all(v2);
		std::sort(v2.begin(), v2.end(), Value_less());

		CHECK_EQUAL(100, v1.size());
		CHECK_EQUAL(100, v2.size());

		for (int i = 0; i < 100; ++i) {
			int ishouldbe = vs[i+vs.size()-100];
			double dshouldbe = ((double)ishouldbe) / (double)RAND_MAX;
			char cc[128];
			sprintf(cc, "ik : %i", ishouldbe);
			std::string sshouldbe(cc);
			
			CHECK_EQUAL(ishouldbe, v1[i]);
			CHECK_EQUAL(sshouldbe, v2[i].sv);
			CHECK_CLOSE(dshouldbe, v2[i].v, 0.0001);
		}
		int ishouldbe = vs[vs.size()-100];
		double dshouldbe = ((double)ishouldbe) / (double)RAND_MAX;
		CHECK_CLOSE(dshouldbe, q1.get_min_key(), 0.0001);
		CHECK_CLOSE(dshouldbe, q2.get_min_key(), 0.0001);
	}
};
