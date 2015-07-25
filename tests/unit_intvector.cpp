/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <sstream>

#include "xasmlib/IntegerVector.h"

#include "Testing.h"
#include "UnitTest++.h"

#include "datamodel/TextIO.h"

using namespace UnitTest;
using namespace std;
using namespace utilities;


namespace {

struct init
{
	init() {
		init_xasmlib();	
	}
} _init;

INT32 count_bits(INT32 i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return ((i + (i >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

#ifdef BITS

#define MX   ((UINT64)MWV::msb * 2l)
#define MSB  ((UINT64)MWV::msb)
#define LSBS ((UINT64)MWV::msb-1)

typedef IntegerVector<BITS> MWV;

TEST(TMNAME(Test_Intvector_AndOr)) {
	//////////////////////////////////////////////////////////////////////////
	// Test 3: 
	// Fill vectors, and, or.
	const int N = max(10, 2*64/BITS);
	MWV mvv(N);
	mvv.zero();
	for (int j = 0; j < N; ++j) {
		CHECK_EQUAL(0, mvv[j]);
	}
	
	mvv |= 7;
	for (int j = 0; j < N; ++j) {
		CHECK_EQUAL(7 & (MX-1), mvv[j]);
	}
	mvv &= 4;
	mvv[1] = 1;	// word is completely contained in the first 32 bits
	mvv[4] = 1;	// word is completely contained in the first 64 bits, but not the first 32 bits
	mvv[9] = 1;	// word is not completely contained in the first 64 bits
	for (int j = 0; j < N; ++j) {
		if (j == 1 || j == 4 || j == 9)	{
			CHECK_EQUAL(1, mvv[j]);
		} else {
			CHECK_EQUAL(4& (MX-1), mvv[j]);
		}
	}	
}

TEST(TMNAME(Test_Intvector_Elem_Assign)) {
	MWV va(1000);

	va.zero();

	for (int i = 0; i < 1000; ++i) {
		CHECK_EQUAL(0, va[i]);
		va[i] = i % MX;
	}

	for (int i = 0; i < 1000; ++i) {
		CHECK_EQUAL(i % MX, va[i]);
	}
}

TEST(TMNAME(Test_Intvector_SetBits)) {
	//////////////////////////////////////////////////////////////////////////
	// Next test: bigger vector, set bits

	MWV va(200);
	MWV vb(200);

	va.zero();
	vb.zero();
	va.set_bits(MSB);
	vb.set_bits(1);
	
	// cout << "a has msb set, b has lsb set" << endl;
	// cout << "a = " << va << endl;
	// cout << "b = " << vb << endl;
	for(size_t j = 0; j < va.size(); ++j) {
		CHECK_EQUAL(MSB, va[j]);
		CHECK_EQUAL(1, vb[j]);
	}

	// cout << "a initially" << endl;
	for(size_t j = 0; j < va.size(); ++j) {
	  va[j] = j & (2*MSB - 1);
	}
	va.bittest(1);
	
	for(size_t j = 0; j < va.size(); ++j) {
		if((j&2) == 0) {
			CHECK_EQUAL(0, va[j]);
		} else if((j&2) == 1) {
			CHECK_EQUAL(MSB, va[j]);
		}
	}
}

#if BITS > 1
TEST(TMNAME(Test_Intvector_Subtract)) {
	MWV va(40);
	MWV vb(40);

	vb.zero();
	for(size_t j = 0; j < va.size(); ++j) {
	  va[j] = j & (2*MSB - 1);
	  vb[j] = rand() & (2*MSB - 1);
	}

	//////////////////////////////////////////////////////////////////////////
	// vector subtraction
	MWV tmp(va);
	tmp -= vb;
	bool carry = false;

	for(int j = 0; j < va.size()-1; ++j) {
		UINT64 av = va[j];
		UINT64 bv = vb[j];
		UINT64 shouldbe;

		if (carry) {
			bv++;
		}

		if(bv <= av) {
			shouldbe = av - bv;
			carry = false;
		} else {
			shouldbe = av + MX - bv;
			carry = true;
		}

		// cout << av << "-" << bv << " = " << shouldbe << "\t" << tmp[j] << endl;
		CHECK_EQUAL(shouldbe, tmp[j]);
	}	
}

TEST(TMNAME(Test_Intvector_Add)) {
	for (int sz = 10; sz < 50; ++sz) {
		MWV va(sz);
		MWV vb(sz);

		for(size_t j = 0; j < va.size(); ++j) {
		  va[j] = MSB-1;
		  vb[j] = 1;
		}
		va[va.size()-1] = 2*MSB-2;
		vb[vb.size()-1] = 2; // produce a carry

		//////////////////////////////////////////////////////////////////////////
		// vector subtraction
		MWV tmp(va);
		tmp += vb;

		for(size_t j = 0; j < va.size(); ++j) {
			UINT64 av = va[j];
			UINT64 bv = vb[j];

			UINT64 shouldbe = j < va.size() - 1 ? MSB : 0;
			if (shouldbe != tmp[j]) {
				cout << "j = " << j << 
					 " " << av << " " << bv << " sz " << va.size() <<  endl;
			}
			CHECK_EQUAL(shouldbe, tmp[j]);
		}
		if( (int)tmp.carry() != 1) {
			cout << "carry fail for size " << sz << endl;
		}
		CHECK_EQUAL(1, (int)tmp.carry());
	}
}

#endif

TEST(TMNAME(Test_Intvector_Saturated_Inc)) {
	using namespace std;
	static const UINT64 vmax  = MSB*2-1;
	static const UINT64 count = min(vmax, (UINT64)1024);


	MWV vb(40);
	for (int i = 0; i < vb.size(); ++i)
	{
		vb[i] = vmax - count;
	}

	//////////////////////////////////////////////////////////////////////////
	// saturated increment
	for(int k = (int)count; k > -5; --k) {
		for(size_t j = 0; j < vb.size(); ++j) {
			if(k > 0) {
				CHECK_EQUAL(vmax - k , vb[j]);
			} else {
				CHECK_EQUAL(vmax, vb[j]);
			}
		}
		vb.saturated_inc();
	}
}

TEST(TMNAME(Test_Intvector_Match_Mask)) {
	//////////////////////////////////////////////////////////////////////////
	// Next test: match masks

	MWV s1(100), s2(100), mask(100);
	for (size_t j = 0; j < 100; ++j) {
		s1[j] = rand() % 4;
		s2[j] = rand() % 4;
	}

	mask.generate_match_mask(s1, s2);

	for (size_t j = 0; j < 100; ++j) {
		if (s1[j] == s2[j]) {
			CHECK_EQUAL((UINT64)MWV::lsbs, mask[j]); 
		} else {
			CHECK_EQUAL(0, mask[j]); 
		}

	}
}

TEST(TMNAME(Test_Intvector_CIPR)) {
	MWV s1(1000), s2(1000), mask(1000);

	//////////////////////////////////////////////////////////////////////////
	// Next test: CIPR addition
	s1.resize(1000);
	s2.resize(1000);
	mask.resize(1000);
	for (int z = 0; z < 100; ++z) {
		for (size_t j = 0; j < 1000; ++j) {
			s1[j] = rand();
			mask[j] = rand();
		}
		s2 = s1;
		
		s1.add_cipr(mask, 0);
		
		BYTE carrybyte = 0;
		UINT64 L = 0; 
		UINT64 M = 0; 
		int bits = 0;
		for (size_t j = 0; j < 1000; ++j) {
		  L = s2[j];
		  M = mask[j];
		  L = L+(L&M) | (L&~M) + carrybyte;
		  carrybyte = ((L & MX) != 0) ? 1 : 0;
		  L&= MX-1;
		  bits+= count_bits((INT32)L);
		  CHECK_EQUAL(L, s1[j]);
		}
		CHECK_EQUAL(bits, s1.count_bits());
	}
}

TEST(TMNAME(Test_Intvector_Cmpxchg_mask)) {
	MWV s1(1000), s2(1000), mask(1000), s1r, s2r;

	//////////////////////////////////////////////////////////////////////////
	// Next test: Seaweed Cmpxchg operation
	for (int z = 0; z < 100; ++z) {
		for (size_t j = 0; j < 1000; ++j) {
			s1[j] = rand();
			s2[j] = rand();
			mask[j] = rand();
		}
		mask.bittest(1);
		
		s1r = s1;
		s2r = s2;
		s1r.cmpxchg_masked(s2r, mask);

		for (int i = 0; i < 1000; ++i) {
			UINT64 ii, jj, m;
			UINT64 ir, jr;

			m = mask[i];
			ii = s1[i];
			jj = s2[i];
			ir = s1r[i];
			jr = s2r[i];

			// std::cout << z << "\t" << i << "\t" << ii<< "\t"  << jj<< "\t"  << m << "\t"
			// 		  << ir << "\t"  << jr << std::endl;

			if(m || ii > jj) {
				// we expect things to be swapped here
				CHECK_EQUAL(ii, jr);
				CHECK_EQUAL(jj, ir);
			} else {
				CHECK_EQUAL(ii, ir);
				CHECK_EQUAL(jj, jr);
			}
		}
	}
}

#endif

};
