/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

// #define _VERBOSETEST

#include "autoconfig.h"

#include <iostream>
#include <sstream>

#include "bsp.h"
#include "xasmlib/IntegerVector.h"

#include "Testing.h"

using namespace std;
using namespace utilities;

const int BITS = 16;
const int MSB = 1 << (BITS-1);
const int LSBS = 2*MSB - 1;

typedef IntegerVector<BITS> MWV;

/************************************************************************/
/* Lots of helpers to reproduce expected behaviour                      */
/************************************************************************/

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
	carry |= ((a & b & 0x8000000000000000LL) != 0) ? 1 : 0;
	*c = (BYTE)carry;
	return a + b;
}

UINT64 extractword_generic(UINT64 * p, BYTE bitofs, BYTE bits) {
	if(bitofs + bits > 64) {
		return (p[0] >> (bitofs & 0x3f)) | 
			   ( (p[1] << (bitofs + bits - 64))
			& (((static_cast<UINT64> (1)) << bits) - 1) );
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

	int N = 1000000;
	bsp_warmup(2);
	{
		// Add with carry benchmark.
		double tt0 = bsp_time();
		INT32 * ra1 = new INT32[N], * ra2 = new INT32[N];
		int a = -1;
		int b = -20;
		BYTE c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry32(a, b, c);
			ra1[z] = a;
		}
		double tt1 = bsp_time();

		a = -1;
		b = -20;
		c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry_generic(a, b, &c);
			ra2[z] = a;
		}
		double tt2 = bsp_time();
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
		double tt0 = bsp_time();
		INT64 * ra1 = new INT64[N], * ra2 = new INT64[N];
		INT64 a = -1;
		INT64 b = -20;
		BYTE c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry64(a, b, c);
			ra1[z] = a;
		}
		double tt1 = bsp_time();

		a = -1;
		b = -20;
		c = 0;
		for (int z = 0; z < N; ++z) {
			a = add_with_carry_generic_64(a, b, &c);
			ra2[z] = a;
		}
		double tt2 = bsp_time();
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
		double tt0 = bsp_time();
		// extractword benchmark
		for (int z = 0; z < N; ++z) {
			UINT64 r1[2] = { (UINT64)rand(), (UINT64)rand() };
			for (int bits = 1; bits < 32; ++bits) {		
				for (int bo = 0; bo < 128-bits; ++bo) {
					extractword(r1, bo, bits);
				}
			}			
		}
		double tt1 = bsp_time();
		// extractword benchmark
		for (int z = 0; z < N; ++z) {
			UINT64 r1[2] = { (UINT64)rand(), (UINT64)rand() };
			for (int bits = 1; bits < 32; ++bits) {		
				for (int bo = 0; bo < 128-bits; ++bo) {
					extractword_generic(r1, bo, bits);
				}
			}			
		}
		double tt2 = bsp_time();
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
		double tt0 = bsp_time();
		vecadd(s1, s2, N, 0);

		double tt1 = bsp_time();
		vecadd_generic(s1a, s2a, N, 0);
		double tt2 = bsp_time();

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
		double tt0 = bsp_time();
		vecadd_cipr(s1, s2, N, 0);

		double tt1 = bsp_time();
		vecadd_generic_cipr(s1a, s2a, N, 0);
		double tt2 = bsp_time();

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
		double tt0 = bsp_time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshl(s1, bshift, N);
		}
		
		double tt1 = bsp_time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshl_generic(s2, bshift, N);
		}
		double tt2 = bsp_time();

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
		double tt0 = bsp_time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshr(s1, bshift, N-2);
		}

		double tt1 = bsp_time();
		for (int bshift = 0; bshift < 32; ++bshift) {
			vecshr_generic(s2, bshift, N-2);
		}
		double tt2 = bsp_time();

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
		double tt0 = bsp_time();
		// TODO: This won't compile unless we have  -D_HAVE_SIMD
		cmpxchg_8_mmx(s1, s2, N);

		double tt1 = bsp_time();
		cmpxchg_generic((BYTE*)s1a, (BYTE*)s2a, sizeof(UINT64)*N);
		double tt2 = bsp_time();

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
