/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

#include "UnitTest++.h"

#include "util/Histogram.h"
#include "bsp_cpp/Shared/SharedVariable.h"


using namespace UnitTest;

namespace {
	TEST(Test_Histogram_Hash)
	{
		using namespace std;
		using namespace utilities;

		Histogram h(1, 2, 5000);

		double actual_values[5000];
		double fixed_values[5000];
		for (int i = 0; i < 5000; ++i) {
			// special cases
			if(i == 0) {
				actual_values[i] = 0.9999;
			} else if(i == 1) {
				actual_values[i] = 1.0001;
			} else if(i == 2) {
				actual_values[i] = 1.9999;
			} else if(i == 3) {
				actual_values[i] = 2.0001;
			} else {
				// generate random values between 0 and 3
				actual_values[i] = (((double) rand())/RAND_MAX) * 3;
			}
			h.add(actual_values[i]);

			if (actual_values[i] < 1) {
				fixed_values[i] = -HUGE_VAL;
			} else if (actual_values[i] > 2) {
				fixed_values[i] = HUGE_VAL;
			} else {
				fixed_values[i] = actual_values[i];
			}
		}

		for (int i = 0; i < 5000; ++i) {
			// if(i < 10) {
			// 	cerr << actual_values[i] << "\t" 
			// 	     << fixed_values[i] << "\t" 
			// 	     << h.unhash(h.hash(actual_values[i])) << endl;
			// }
			CHECK_CLOSE(fixed_values[i], 
				h.unhash(h.hash(actual_values[i])), 1e-3);
		}		
	}

	TEST(Test_Histogram_Copy) {
		using namespace utilities;
		Histogram h(1, 2, 5000);
		double actual_values[5000];
		double fixed_values[5000];
		for (int i = 0; i < 5000; ++i) {
			actual_values[i] = (((double) rand())/RAND_MAX) * 3;
			h.add(actual_values[i]);

			if (actual_values[i] < 1) {
				fixed_values[i] = -HUGE_VAL;
			} else if (actual_values[i] > 2) {
				fixed_values[i] = HUGE_VAL;
			} else {
				fixed_values[i] = actual_values[i];
			}
		}

		Histogram h2(h);
		Histogram h3 = h;

		CHECK_EQUAL(h.min_val, h2.min_val);
		CHECK_EQUAL(h.min_val, h3.min_val);

		CHECK_EQUAL(h.max_val, h2.max_val);
		CHECK_EQUAL(h.max_val, h3.max_val);

		CHECK_EQUAL(h.nbuckets, h2.nbuckets);
		CHECK_EQUAL(h.nbuckets, h3.nbuckets);

		for (int i = 0; i < 5000; ++i) {
			CHECK_EQUAL(h.count(actual_values[i]), h2.count(actual_values[i]));
			CHECK_EQUAL(h.count(actual_values[i]), h3.count(actual_values[i]));
		}

		for (int i = 0; i < h.nbuckets+2; ++i) {
			CHECK_EQUAL(h.buckets[i], h2.buckets[i]);
			CHECK_EQUAL(h.buckets[i], h3.buckets[i]);
		}
	}

	TEST(Test_Histogram_RW) {
		using namespace utilities;
		using namespace std;
		
		Histogram h(1, 2, 5000);
		double actual_values[5000];
		for (int i = 0; i < 5000; ++i) {
			actual_values[i] = (((double) rand())/RAND_MAX) * 3;
			h.add(actual_values[i]);
		}

		Histogram h2;

		std::string ser;
		{
			std::ostringstream oss;
			h.write(oss);
			ser = oss.str();
		}
		{
			std::istringstream in(ser);
			h2.read(in);
		}

		CHECK_EQUAL(h.min_val, h2.min_val);

		CHECK_EQUAL(h.max_val, h2.max_val);

		CHECK_EQUAL(h.nbuckets, h2.nbuckets);

		for (int i = 0; i < h.nbuckets+2; ++i) {
			CHECK_EQUAL(h.buckets[i], h2.buckets[i]);
		}

		for (int i = 0; i < 5000; ++i) {
			// cout << h.count(actual_values[i])  << "\t" <<  h2.count(actual_values[i]) << endl;
			CHECK_EQUAL(h.count(actual_values[i]), h2.count(actual_values[i]));
		}

	}

	TEST(Test_Histogram_Shared_Serialise) {
		using namespace utilities;
		using namespace std;

		Histogram h(1, 2, 5000);
		double actual_values[5000];
		for (int i = 0; i < 5000; ++i) {
			actual_values[i] = (((double) rand())/RAND_MAX) * 3;
			h.add(actual_values[i]);
		}

		Histogram h2;

		bsp::SharedVariable< Histogram, bsp::Reduce<Histogram> > 
			shv(h);
		bsp::SharedVariable< Histogram, bsp::Reduce<Histogram> >  
			shv2(h2);

		char * data= new char[shv.serialized_size()];
		
		shv.serialize(data, shv.serialized_size());
		shv2.deserialize(data, shv.serialized_size());

		delete [] data;

		CHECK_EQUAL(h.min_val, h2.min_val);

		CHECK_EQUAL(h.max_val, h2.max_val);

		CHECK_EQUAL(h.nbuckets, h2.nbuckets);

		for (int i = 0; i < h.nbuckets+2; ++i) {
			CHECK_EQUAL(h.buckets[i], h2.buckets[i]);
		}

		for (int i = 0; i < 5000; ++i) {
			// cout << h.count(actual_values[i])  << "\t" <<  h2.count(actual_values[i]) << endl;
			CHECK_EQUAL(h.count(actual_values[i]), h2.count(actual_values[i]));
		}
	}

	TEST(Test_Histogram_Combine) {
		using namespace utilities;
		using namespace std;

		// make a histogram.
		Histogram h(1, 2, 5000);

		// and 5 histograms which together should give the same as above
		Histogram hset[5];

		for(int j = 0; j < 5; ++j) {
			hset[j].reset(1,2,5000);
		}

		double actual_values[5000];
		for (int i = 0; i < 5000; ++i) {
			actual_values[i] = (((double) rand())/RAND_MAX) * 3;
			h.add(actual_values[i]);

			hset[i%5].add(actual_values[i]);
		}
		
		Histogram h2(1,2, 5000);

		for(int j = 0; j < 5; ++j) {
			h2.reduce_with(&(hset[j]));
		}

		CHECK_EQUAL(h.min_val, h2.min_val);
		CHECK_EQUAL(h.max_val, h2.max_val);
		CHECK_EQUAL(h.nbuckets, h2.nbuckets);

		for (int i = 0; i < h.nbuckets+2; ++i) {
			CHECK_EQUAL(h.buckets[i], h2.buckets[i]);
		}
	}

};
