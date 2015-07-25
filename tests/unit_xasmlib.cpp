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
/************************************************************************/
/* Some helpers to reproduce expected behaviour                         */
/************************************************************************/

INT32 count_bits(INT32 i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return ((i + (i >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

TEST(Test_Add_with_carry_bittest) {
	//////////////////////////////////////////////////////////////////////////
	// Test 1: Add with carry.
	// -1 + 1 = 0 carry 1
	int a=-1, b=1;
	INT64 a1=-1, b1=1;
	BYTE c=1;

	a = add_with_carry32(a, b, c);	
	CHECK (a == 1 && c == 1);
	
	c = 1;
	a1 = add_with_carry64(a1, b1, c);
	CHECK (a == 1 && c == 1);

	//////////////////////////////////////////////////////////////////////////
	// Test 2: 
	// set all bits
	a1 = -1;
	// reset bit 2
	a1 &= (~4);
	// reset bit 34
	a1 &= 0xfffffffbffffffffll;

	CHECK( bittest64(a1, 1) && !bittest64(a1, 2) && !bittest64(a1, 34) );
}

TEST(Test_Intvector_StreamIO) {
	int N = 500;
	// vector stream i/o test
	// more than 64-bits overall
	IntegerVector<2> twobitvec (N);
	IntegerVector<6> sixbitvec (N);
	IntegerVector<16> sixteenbitvec (N);
	IntegerVector<2> twobitvec2 ;
	IntegerVector<6> sixbitvec2;
	IntegerVector<16> sixteenbitvec2;

	// generate test sequences
	for (size_t j = 0; j < N; ++j) {
		twobitvec[j] = j & 3;
		sixbitvec[j] = j & 63;
		sixteenbitvec[j] = j & 0xffff;
	}

	{
		stringstream mystream;
		mystream << twobitvec;
		mystream >> twobitvec2;
	}

	{
		stringstream mystream;
		mystream << sixbitvec;
		mystream >> sixbitvec2;
	}

	{
		stringstream mystream;
		mystream << sixteenbitvec;
		mystream >> sixteenbitvec2;
	}

	CHECK_EQUAL (twobitvec.size(), twobitvec2.size());
	CHECK_EQUAL (sixbitvec.size(), sixbitvec.size());
	CHECK_EQUAL (sixteenbitvec.size(), sixteenbitvec.size());

	for (size_t j = 0; j < N; ++j) {
		CHECK_EQUAL(twobitvec[j], twobitvec2[j]);
		CHECK_EQUAL(sixbitvec[j], sixbitvec[j]);
		CHECK_EQUAL(sixteenbitvec[j], sixteenbitvec[j]);
	}
}

TEST(Test_Intvector_Shift) {
	using namespace std;
	IntegerVector<8> va(40);
	const UINT64 MSB = 0x80;

	//////////////////////////////////////////////////////////////////////////
	// shifting
	for(size_t j = 0; j < va.size(); ++j) {
	  va[j] = j;
	}

	va <<= 8;
	int bitcount = 0;
	for(size_t j = 0; j < va.size(); ++j) {
		bitcount += count_bits((INT32)va[j]);
		CHECK_EQUAL(max(0, (int)j-1), va[j]);
	}
	// This tests fixending
	CHECK_EQUAL(bitcount, va.count_bits());

	va <<= 32;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe =  max((int)j-5, 0);
	  CHECK_EQUAL(shouldbe, va[j]);
	}

	va <<= 40;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = max((int)j-10, 0);
	  CHECK_EQUAL(shouldbe, va[j]);
	}

	va >>= 8;

	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = 
	    max((int)j-9, 0);
	  if(j >= 39) {
	    shouldbe = 0;
	  }
	  CHECK_EQUAL(shouldbe, va[j]);
	}
	va >>= 32;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = max((int)j-5, 0);
	  if(j >= 35) {
	    shouldbe = 0;
	  }
	  CHECK_EQUAL(shouldbe, va[j]);
	}
	va >>= 40;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = (int)j;
	  if(j >= 30) {
	    shouldbe = 0;
	  }
	  CHECK_EQUAL(shouldbe, va[j]);
	}
}


};
