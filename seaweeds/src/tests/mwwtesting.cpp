/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#define _VERBOSETEST

#include "autoconfig.h"

#include <iostream>

#include "bspcpp/tools/utilities.h"
#include "xasmlib/IntegerVector.h"

#include "tests/Testing.h"

using namespace std;
using namespace utilities;

const int BITS = 16;
const int MSB = 1 << (BITS-1);
const int LSBS = 2*MSB - 1;

typedef IntegerVector<BITS> MWV;

INT32 count_bits(INT32 i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return ((i + (i >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

INT32 add_with_carry_generic(INT32 a, INT32 b, BYTE * c) {
	INT32 carry = 0;
	// the first time we might create an overflow is when
	// adding the carry from *c 
	carry = a == -1;
	a+= (INT32)*c;

	// if both msbs are set, we also get an overflow
	carry |= ((a & b & 0x80000000) != 0) ? 1 : 0;
	*c = carry;
	return a + b;
}

INT64 add_with_carry_generic_64(INT64 a, INT64 b, BYTE * c) {
	INT64 carry = 0;
	// the first time we might create an overflow is when
	// adding the carry from *c 
	carry = a == -1;
	a+= (INT64)*c;

	// if both msbs are set, we also get an overflow
	carry |= ((a & b & 0x8000000000000000) != 0) ? 1 : 0;
	*c = carry;
	return a + b;
}

UINT64 extractword_generic(UINT64 * p, BYTE bitofs, BYTE bits) {
	if(bitofs + bits > 64) {
		return (p[0] >> (bitofs & 0x3f)) | 
			   (p[1] << (bitofs + bits - 64))
			& (((static_cast<UINT64> (1)) << bits) - 1);
	} else {
		return (p[0] >> (bitofs & 0x3f)) & (((static_cast<UINT64> (1)) << bits) - 1);
	}
}

void vecadd_generic(UINT64 * data1, UINT64 * data2, size_t len, BYTE initial_carry /* = 0 */) {
	BYTE carry = initial_carry;
	for (size_t z = 0; z < len; ++z) {
		*data1 = add_with_carry64(*data1, *data2, carry);
		++data1; ++data2;
	}
}

void vecadd_generic_cipr(UINT64 * M, UINT64 * L, size_t len, BYTE initial_carry /* = 0 */) {
	BYTE carry = initial_carry;
	for (size_t z = 0; z < len; ++z) {
		*L = add_with_carry64(*L & *M, *L & (~(*M)), carry);
		++M; ++L;
	}
}

void vecshl_generic(UINT64 * data, size_t shift, size_t len) {
	UINT64 tmp1 = 0, tmp2;
	for (size_t z = 0; z < len; ++z) {
		tmp2 = *data;
		*data = (*data << shift) | (tmp1 >> (64 - shift));
		tmp1 = tmp2;
		++data;
	}
}

void vecshr_generic(UINT64 * data, size_t shift, size_t len) {
	for (size_t z = 0; z < len; ++z) {
		*data = (*data >> shift) | (data[1] << (64 - shift));
		++data;
	}
}

void cmpxchg_generic(BYTE * data1, BYTE * data2, size_t len) {
	for (size_t z = 0; z < len; ++z) {
		BYTE t1 = *data1;
		BYTE t2 = *data2;
		if(t1 > t2) {
			*data1 = t2;
			*data2 = t1;
		}
	}	
}

int main(int argc, char *argv[]) {	
	using namespace std;
	init_xasmlib();

	//////////////////////////////////////////////////////////////////////////
	// Test 1: Add with carry.
	// -1 + 1 = 0 carry 1
	int a=-1, b=1;
	INT64 a1=-1, b1=1;
	BYTE c=1;

	a = add_with_carry32(a, b, c);	
	if (a != 1 || c != 1) {
		cout << "FAIL: " << "a = add_with_carry32(a, b, c);	";
		cout << a << ", " << b << ", " << (int)c << endl;
	}
	
	c = 1;
	a1 = add_with_carry64(a1, b1, c);
	if (a1 != 1 || c != 1) {
		cout << "FAIL: " << "a = add_with_carry64(a1, b1, c);	";
			cout << a1 << ", " << b1 << ", " << (int)c << endl;
	}

	//////////////////////////////////////////////////////////////////////////
	// Test 2: 
	// set all bits
	a1 = -1;
	// reset bit 2
	a1 &= (~4);
	// reset bit 34
	a1 &= 0xfffffffbffffffffll;

	if(!( bittest64(a1, 1) && !bittest64(a1, 2) && !bittest64(a1, 34))) {
		cout << "FAIL: " << "!( bittest64(a1, 1) && !bittest64(a1, 2) && !bittest64(a1, 34))   ";
		cout << hex << a1 << dec <<  ", " << (int)bittest64(a1, 1) << ", " <<
			(int)bittest64(a1, 2) << ", " << (int)bittest64(a1, 34) << endl;
	}
	

	//////////////////////////////////////////////////////////////////////////
	// Test 3: 
	// Fill vectors, and, or.
	IntegerVector<7> mvv(10);
	mvv.zero();
	for (int j = 0; j < 10; ++j) {
		if(mvv[j] != 0) {
			cout << "FAIL: zero(), "  << j << endl;
		}
	}
	
	mvv |= 7;
	for (int j = 0; j < 10; ++j) {
		if(mvv[j] != 7) {
			cout << "FAIL: or 7, "  << j << endl;
		}
	}
	mvv &= 4;
	mvv[4] = 29;
	for (int j = 0; j < 10; ++j) {
		if (j == 4)	{
			if(mvv[j] != 29) {
				cout << "FAIL: insert 29 at pos 4 "<< endl;
			}
		} else if(mvv[j] != 4) {
			cout << "FAIL: and 4, "  << j << endl;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Next test: bigger vector, set bits
	MWV va(40);
	MWV vb(40);

	va.zero();
	vb.zero();
	va.set_bits(MSB);
	vb.set_bits(1);
	
	// cout << "a has msb set, b has lsb set" << endl;
	// cout << "a = " << va << endl;
	// cout << "b = " << vb << endl;
	for(size_t j = 0; j < 40; ++j) {
		if (va[j] != MSB) {
			cout << "FAIL: set_bits(MSB)" << j << endl;
		}
		if (vb[j] != 1) {
			cout << "FAIL: set_bits(LSB)" << j << endl;
		}
	}

	// cout << "a initially" << endl;
	for(size_t j = 0; j < va.size(); ++j) {
	  va[j] = j & (2*MSB - 1);
	}
	va.bittest(1);
	
	bool f0 = false;
	for(size_t j = 0; j < va.size(); ++j) {
	  if((j&2) == 0) {
	    if(va[j] != 0) {
	      cout << "FAIL: bit_test pos " << j << endl;
	      f0 = true;
	    }
	  } else if((j&2) == 1) {
	    if(va[j] != MSB) {
	      cout << "FAIL: bit_test pos " << j << endl;
	      f0 = true;
	    }
	  }
	}
	if(f0) {
	  cout << "FAIL: bit_test: a= " << va << endl;
	}

	//////////////////////////////////////////////////////////////////////////
	// Next test: vector subtraction
	MWV tmp(va);
	tmp -= vb;

	for(size_t j = 0; j < va.size(); ++j) {
	  if((j&3) == 0) {
	    if(tmp[j] != LSBS) {
	      cout << "FAIL: a-b, " << j << " -- " << tmp[j] << " != " << LSBS << endl;
	    }
	  } 
	  if((j&3) == 1) {
	    if(tmp[j] != LSBS - 1) {
	      cout << "FAIL: a-b, " << j << " -- " << tmp[j] << " != " << LSBS - 1 << endl;
	    }
	  } 
	  if((j&3) == 2) {
	    if(tmp[j] != LSBS - 2) {
	      cout << "FAIL: a-b, " << j << " -- " << tmp[j] << " != " << LSBS - 2 << endl;
	    }
	  } 
	  if((j&3) == 3) {
	    if(tmp[j] != LSBS - 1) {
	      cout << "FAIL: a-b, " << j << " -- " << tmp[j] << " != " << LSBS - 1 << endl;
	    }
	  }
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Next test: saturated increment
	for(size_t k = 0; k < (1 << BITS); ++k) {
		vb.saturated_inc();
		for(size_t j = 0; j < vb.size(); ++j) {
		  if(vb[j] != min(k+2, (size_t)LSBS)) {
		    cout << "Saturated inc fail j = " << j << " vb[j] = " << vb[j] << " (" << k+2 <<")" << endl;
		  }
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Next test: shifting
	for(size_t j = 0; j < va.size(); ++j) {
	  va[j] = j & (2*MSB - 1);
	}
	va <<= 8;
	int bitcount = 0;
	for(size_t j = 0; j < va.size(); ++j) {
	  bitcount += count_bits(va[j]);
	  if(va[j] != (j & (2*MSB - 1)) << 8) {
	    cout << "FAIL: << 8 " << j << endl;
	  }
	}
	// This tests fixending
	if(bitcount != va.count_bits()) {
	  cout << "FAIL: bitcount " << bitcount << " != " << va.count_bits() << endl;
	}

	va <<= 32;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = 
	    max((int)j-2, 0) << 8;
	  if(va[j] != shouldbe) {
	    cout << "FAIL: << 32 " << j << " value " << hex << va[j] << " != " << hex << shouldbe << endl;
	  }
	}

	va <<= 40;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = 
	    max((int)j-5, 0);
	  if(va[j] != shouldbe) {
	    cout << "FAIL: << 40 " << j << " value " << hex << va[j] << " != " << hex << shouldbe << endl;
	  }
	}

	va >>= 8;

	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = 
	    max((int)j-4, 0) << 8;
	  if(j >= 39) {
	    shouldbe = 0;
	  }
	  if(va[j] != shouldbe) {
	    cout << "FAIL: << 40 >> 8 " << j << " value " << hex << va[j] << " != " << hex << shouldbe << endl;
	  }
	}
	va >>= 32;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = 
	    max((int)j-2, 0) << 8;
	  if(j >= 37) {
	    shouldbe = 0;
	  }
	  if(va[j] != shouldbe) {
	    cout << "FAIL: << 40 >> 32 " << j << " value " << hex << va[j] << " != " << hex << shouldbe << endl;
	  }
	}
	va >>= 40;
	for(size_t j = 0; j < va.size(); ++j) {
	  int shouldbe = j;
	  if(j >= 35) {
	    shouldbe = 0;
	  }
	  if(va[j] != shouldbe) {
	    cout << "FAIL: << 40 >> 40 " << j << " value " << hex << va[j] << " != " << hex << shouldbe << endl;
	  }
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Next test: match masks

	IntegerVector<8> s1(100), s2(100), mask(100);
	for (size_t j = 0; j < 100; ++j) {
		s1[j] = rand() % 4;
		s2[j] = rand() % 4;
	}

	mask.generate_match_mask(s1, s2);

	for (size_t j = 0; j < 100; ++j) {
		if (s1[j] == s2[j]) {
			if( mask[j] != IntegerVector<8>::lsbs ) {
				cout << "FAIL: Mismatch mask not -1: "<< j << " " << mask[j] << " " << " " << IntegerVector<8>::lsbs << endl;
			}
		} else {
			if( mask[j] != 0 ) {
				cout << "Mismatch mask not 0: "<< j << endl;
			}
		}

	}

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
		  carrybyte = ((L & 0x100) != 0) ? 1 : 0;
		  L&= 0xFF;
		  bits+= count_bits(L);
		  if(L != s1[j]) {
			cout << "FAIL: CIPR " << j << " L = " << L << " s1 = "<< s1[j] << endl;
		  }
		}
		if(bits != s1.count_bits()) {
			cout << "FAIL: bitcount " << bits << " != " << s1.count_bits() << endl;
		}
	}
	
	int N = 1000000;
	utilities::warmup();
	{
		// Add with carry benchmark.
		double tt0 = utilities::time();
		INT32 * ra1 = new INT32[N], * ra2 = new INT32[N];
		int a = -1;
		int b = -20;
		BYTE c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry32(a, b, c);
			ra1[z] = a;
		}
		double tt1 = utilities::time();

		a = -1;
		b = -20;
		c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry_generic(a, b, &c);
			ra2[z] = a;
		}
		double tt2 = utilities::time();
		for (int z = 0; z < N; ++z) {
			if(ra1[z] != ra2[z]) {
				cout << "Mismatch at location " << z << ", " << ra1[z] << " != " << ra2[z] << endl;
//				exit(0);
			}
		}
		cout << "Time for adding with carry ("<< N << " 32-bit integers)" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;
		delete [] ra1;
		delete [] ra2;
	}
	{
		// Add with carry benchmark, 64 bit.
		double tt0 = utilities::time();
		INT64 * ra1 = new INT64[N], * ra2 = new INT64[N];
		INT64 a = -1;
		INT64 b = -20;
		BYTE c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry64(a, b, c);
			ra1[z] = a;
		}
		double tt1 = utilities::time();

		a = -1;
		b = -20;
		c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry_generic_64(a, b, &c);
			ra2[z] = a;
		}
		double tt2 = utilities::time();
		for (int z = 0; z < N; ++z) {
			if(ra1[z] != ra2[z]) {
				cout << "Mismatch at location " << z << ", " << ra1[z] << " != " << ra2[z] << endl;
				//				exit(0);
			}
		}
		cout << "Time for adding with carry ("<< N << " 64-bit integers)" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;
		delete [] ra1;
		delete [] ra2;
	}
	N = 10000;
	{
		double tt0 = utilities::time();
		// extractword benchmark
		for (int z = 0; z < N; ++z) {
			UINT64 r1[2] = { rand(), rand() };
			for (int bits = 1; bits < 32; ++bits) {		
				for (int bo = 0; bo < 128-bits; ++bo) {
					extractword(r1, bo, bits);
				}
			}			
		}
		double tt1 = utilities::time();
		// extractword benchmark
		for (int z = 0; z < N; ++z) {
			UINT64 r1[2] = { rand(), rand() };
			for (int bits = 1; bits < 32; ++bits) {		
				for (int bo = 0; bo < 128-bits; ++bo) {
					extractword_generic(r1, bo, bits);
				}
			}			
		}
		double tt2 = utilities::time();
		cout << "Time for extracting " << 32*N*16 << " words" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;
	}

	N = 100000;
	{	// vector addition benchmark
		UINT64 * s1 = new UINT64[N], * s2 = new UINT64[N];
		UINT64 * s1a = new UINT64[N], * s2a = new UINT64[N];
		for (int z = 0; z < N; ++z) {
			s1[z] = s1a[z] = -z;
			s2[z] = s2a[z] = 0x100 - z;
		}
		double tt0 = utilities::time();
		vecadd(s1, s2, N, 0);

		double tt1 = utilities::time();
		vecadd_generic(s1a, s2a, N, 0);
		double tt2 = utilities::time();

		for (int z = 0; z < N; ++z) {
			if (s1[z] != s1a[z]) {
				cout << "Mismatch at location " << z << ", " << s1[z] << " != " << s1a[z] << endl;
			}
		}
		

		cout << "Time for vec-adding " << N << " words" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;

		delete [] s1;
		delete [] s2;
		delete [] s1a;
		delete [] s2a;
	}

	{	// vector LCS addition benchmark
		UINT64 * s1 = new UINT64[N], * s2 = new UINT64[N];
		UINT64 * s1a = new UINT64[N], * s2a = new UINT64[N];
		for (int z = 0; z < N; ++z) {
			s1[z] = s1a[z] = -z;
			s2[z] = s2a[z] = 0x100 - z;
		}
		double tt0 = utilities::time();
		vecadd_cipr(s1, s2, N, 0);

		double tt1 = utilities::time();
		vecadd_generic_cipr(s1a, s2a, N, 0);
		double tt2 = utilities::time();

		for (int z = 0; z < N; ++z) {
			if (s1[z] != s1a[z]) {
				cout << "Mismatch at location " << z << ", " << s1[z] << " != " << s1a[z] << endl;
			}
		}


		cout << "Time for LCS CIPR vec-adding " << N << " words" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;

		delete [] s1;
		delete [] s2;
		delete [] s1a;
		delete [] s2a;
	}

	{	// vector shift benchmark
		UINT64 * s1 = new UINT64[N], * s2 = new UINT64[N];
		for (int z = 0; z < N; ++z) {
			s1[z] = -1;
			s2[z] = -1;
		}
		double tt0 = utilities::time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshl(s1, bshift, N);
		}
		
		double tt1 = utilities::time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshl_generic(s2, bshift, N);
		}
		double tt2 = utilities::time();

		for (int z = 0; z < N; ++z) {
			if (s1[z] != s2[z]) {
				cout << "Mismatch at location " << z << ", " << s1[z] << " != " << s2[z] << endl;
			}
		}
		
		cout << "Time for bit-shifting " << N << " words" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;

		delete [] s1;
		delete [] s2;
	}

	{	// vector shift benchmark 2
		UINT64 * s1 = new UINT64[N], * s2 = new UINT64[N];
		for (int z = 0; z < N-2; ++z) {
			s1[z] = -1;
			s2[z] = -1;
		}
		s1[N-1] = 0;
		s2[N-1] = 0;
		s1[N-2] = 0;
		s2[N-2] = 0;
		double tt0 = utilities::time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshr(s1, bshift, N-2);
		}

		double tt1 = utilities::time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshr_generic(s2, bshift, N-2);
		}
		double tt2 = utilities::time();

		for (int z = 0; z < N-10; ++z) {
			if (s1[z] != s2[z]) {
				cout << "Mismatch at location " << z << ", " << s1[z] << " != " << s2[z] << endl;
			}
		}

		cout << "Time for bit-shifting " << N << " words" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;

		delete [] s1;
		delete [] s2;
	}

	{	// vector cmpxchg benchmark
		UINT64 * s1 = new UINT64[N], * s2 = new UINT64[N];
		UINT64 * s1a = new UINT64[N], * s2a = new UINT64[N];
		for (int z = 0; z < N; ++z) {
			s1[z] = s1a[z] = rand();
			s2[z] = s2a[z] = rand();
		}
		double tt0 = utilities::time();
		cmpxchg_8_mmx(s1, s2, N);

		double tt1 = utilities::time();
		cmpxchg_generic((BYTE*)s1a, (BYTE*)s2a, sizeof(UINT64)*N);
		double tt2 = utilities::time();

/*
		for (int z = 0; z < N; ++z) {
			if (s1[z] != s1a[z]) {
				cout << "s1a Mismatch at location " << z << ", " << s1[z] << " != " << s1a[z] << endl;
			}
			if (s2[z] != s2a[z]) {
				cout << "s2a Mismatch at location " << z << ", " << s2[z] << " != " << s2a[z] << endl;
			}
		}
*/

		cout << "Time for cmpxchg operation " << N << " words" << endl;
		cout << "  Generic: " << tt2-tt1 << " seconds" << endl;
		cout << "  Assembler: " << tt1-tt0 << " seconds" << endl;

		delete [] s1;
		delete [] s2;
		delete [] s1a;
		delete [] s2a;
	}

	return EXIT_SUCCESS;
}
