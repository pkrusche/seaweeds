/***************************************************************************
*   Copyright (C) 2009 by Peter Krusche                                   *
*   pkrusche@gmail.com                                                    *
***************************************************************************/

#include <stdlib.h>

#include "autoconfig.h"

#ifndef _WIN32
#define __cdecl
#endif

extern void __cdecl do_emms();
extern void __cdecl initbitmasks();

UINT64 bits_64[64];

void exit_xasmlib(void) {
#ifndef _NO_MMX
	do_emms();
#endif
}

void init_xasmlib() {
	int i;
	UINT64 x = 1;
	initbitmasks();

	for (i = 0; i < 64; ++i)
	{
		bits_64[i] = x;
		x <<= 1;
	}

	atexit(exit_xasmlib);
}

#ifdef _X86_64
#ifndef _WIN32

#include <sys/types.h>

u_int64_t extractword(u_int64_t * p, u_char bitofs, u_char bits) {
	if(bitofs + bits > 64) {
		u_int64_t mask = (unsigned)-1LL >> (128 - bitofs - bits);
		return (p[0] >> (bitofs & 0x3f)) | 
			   ( (p[1]&mask) << (64-bitofs & 0x3f));
	} else {
		return (p[0] >> (bitofs & 0x3f)) & ((1LL << bits) - 1);
	}
}

void insertword(u_int64_t val, u_int64_t * p, u_char bitofs, u_char bits) {
	if(bitofs + bits > 64) {
		u_int64_t mask1 = (1LL << bitofs) - 1;
		u_int64_t mask2 = -1LL << (bitofs + bits - 64);
		p[0] = (p[0] & mask1) | (val << (bitofs & 0x3f));
		p[1] = (p[1] & mask2) | (val >> (64-bitofs));
	} else if(bitofs + bits == 64) {
		u_int64_t mask = ((1LL << bitofs) - 1);
		p[0] = (p[0] & mask) | (val << (bitofs & 0x3f));
	} else {
		u_int64_t mask = ((1LL << bitofs) - 1)
					| 	 (-1LL << (bitofs + bits));
		p[0] = (p[0] & mask) | (val << (bitofs & 0x3f));
	}
}

#endif
#endif

/** len is in bytes! */
void cmpxchg_with_mask_8(BYTE * data, BYTE * data2, BYTE * mask, size_t len) {
	size_t j;
	for (j= 0; j < len; ++j) {
		BYTE tmp1 = data[j];
		BYTE tmp2 = data2[j];
		BYTE tmp3 = mask[j];
		if (tmp3 || tmp1 > tmp2) {
			data[j] = tmp2;
			data2[j] = tmp1;
		}
	}
}

/** len is in words */
void cmpxchg_with_mask_16(WORD * data, WORD * data2, WORD * mask, size_t len) {
	size_t j;
	for (j= 0; j < len; ++j) {
		WORD tmp1 = data[j];
		WORD tmp2 = data2[j];
		WORD tmp3 = mask[j];
		if (tmp3 || tmp1 > tmp2) {
			data[j] = tmp2;
			data2[j] = tmp1;
		}
	}
}

