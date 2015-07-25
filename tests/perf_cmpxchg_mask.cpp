/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

// #define _VERBOSETEST

#include "autoconfig.h"

#include <iostream>

#include "util/TypeList.h"

#include "bsp_tools/utilities.h"
#include "xasmlib/IntegerVector.h"
#include "tuning/Timing.h"

using namespace std;
using namespace utilities;
using namespace tuning;

int g_oversample = 10000;

template<
	int _bpc
>
class Mask_Benchmark {
public:
	Mask_Benchmark(size_t )  {
	}

	double operator() (size_t size) {
		using namespace utilities;
		IntegerVector<_bpc> text1(size);
		IntegerVector<_bpc> text2(size);
		IntegerVector<_bpc> mask(size);

		for(size_t j = 0; j < size; ++j) {
			text1[j] = rand() & 3;
			text2[j] = rand() & 3;
		}

		mask.test_zero();

		double t0 = bsp_time();

		for (int z = 0; z < g_oversample; ++z) {
			mask.generate_match_mask(text1, text2);
		}

		double t = bsp_time();

		return (t - t0)/g_oversample;
	}
};

template<
	int _bpc
>
class Cmpxchg_Benchmark {
public:
	Cmpxchg_Benchmark(size_t )  {
	}

	double operator() (size_t size) {
		using namespace utilities;
		IntegerVector<_bpc> text1(size);
		IntegerVector<_bpc> text2(size);

		for(size_t j = 0; j < size; ++j) {
			text1[j] = rand() & 3;
			text2[j] = rand() & 3;
		}

		double t0 = bsp_time();

		for (int z = 0; z < g_oversample; ++z) {
			text1.cmpxchg(text2);
		}

		double t = bsp_time();

		return (t - t0)/g_oversample;
	}
};


template<
    int _bpc
>
class Cmpxchg_Mask_Benchmark {
public:
	Cmpxchg_Mask_Benchmark(size_t )  {
 	}
	
    double operator() (size_t size) {
        using namespace utilities;
		IntegerVector<_bpc> text1(size);
		IntegerVector<_bpc> text2(size);
		IntegerVector<_bpc> mask(size);

		for(size_t j = 0; j < size; ++j) {
			text1[j] = rand() & 3;
			text2[j] = rand() & 3;
			mask[j] = rand() & 1;
		}

		mask.test_zero();
	        
		double t0 = bsp_time();

		for (int z = 0; z < g_oversample; ++z) {
			text1.cmpxchg_masked(text2, mask);
		}
		
		double t = bsp_time();
        
        return (t - t0)/g_oversample;
    }
};

typedef tuning::Timing < Cmpxchg_Benchmark<4>, size_t > Cmpxchg_HALFBYTE;
typedef tuning::Timing < Cmpxchg_Benchmark<8>, size_t > Cmpxchg_BYTE;
typedef tuning::Timing < Cmpxchg_Benchmark<16>, size_t > Cmpxchg_WORD;
typedef tuning::Timing < Cmpxchg_Benchmark<32>, size_t > Cmpxchg_DWORD;

typedef tuning::Timing < Cmpxchg_Mask_Benchmark<4>, size_t > Cmpxchg_Mask_HALFBYTE;
typedef tuning::Timing < Cmpxchg_Mask_Benchmark<8>, size_t > Cmpxchg_Mask_BYTE;
typedef tuning::Timing < Cmpxchg_Mask_Benchmark<16>, size_t > Cmpxchg_Mask_WORD;
typedef tuning::Timing < Cmpxchg_Mask_Benchmark<32>, size_t > Cmpxchg_Mask_DWORD;


typedef tuning::Timing < Cmpxchg_Mask_Benchmark<4>, size_t > Genmask_HALFBYTE;
typedef tuning::Timing < Cmpxchg_Mask_Benchmark<8>, size_t > Genmask_BYTE;
typedef tuning::Timing < Cmpxchg_Mask_Benchmark<16>, size_t > Genmask_WORD;
typedef tuning::Timing < Cmpxchg_Mask_Benchmark<32>, size_t > Genmask_DWORD;

typedef TYPELIST_4(Cmpxchg_HALFBYTE, Cmpxchg_BYTE, Cmpxchg_WORD, Cmpxchg_DWORD) 
	Cmpxchg_algorithms;

typedef TYPELIST_4(Cmpxchg_Mask_HALFBYTE, Cmpxchg_Mask_BYTE, Cmpxchg_Mask_WORD, Cmpxchg_Mask_DWORD) 
	Cmpxchg_Mask_algorithms;

typedef TYPELIST_4(Genmask_HALFBYTE, Genmask_BYTE, Genmask_WORD, Genmask_DWORD) 
	Genmask_algorithms;

int main(int argc, char *argv[]) {

	enum {
		cx,
		cxm,
		gm
	} type = cxm;
	int add = 1000;
	int NUM = 20;
	int inc = 1000;

	init_xasmlib();

	if (argc > 1) {
		if (strcmp(argv[1], "cx") == 0) {
			type = cx;
		} else if (strcmp(argv[1], "cxm") == 0) {
			type = cxm;
		} else if (strcmp(argv[1], "gm") == 0) {
			type = gm;
		}
	}
	

	if(argc > 2) {
		add = atoi(argv[2]);
	}

	if(argc > 2) {
		add = atoi(argv[2]);
	}

	if(argc > 3) {
		NUM = atoi(argv[3]);
	}

	if(argc > 4) {
		inc = atoi(argv[4]);
	}

	if(argc > 5) {
		g_oversample = atoi(argv[5]);
	}

	TimingList tdata; 

	switch (type) {
	case cx:
		MultiTiming < 
			size_t, 
			Cmpxchg_algorithms
		>::getTimings(add, NUM, inc, tdata, 0);
		break;
	case cxm:
		MultiTiming < 
			size_t, 
			Cmpxchg_Mask_algorithms
		>::getTimings(add, NUM, inc, tdata, 0);
		break;
	case gm:
		MultiTiming < 
			size_t, 
			Genmask_algorithms
		>::getTimings(add, NUM, inc, tdata, 0);
		break;
	default:
		break;
	}
	
	cout << tdata << endl;

	return EXIT_SUCCESS;
}

