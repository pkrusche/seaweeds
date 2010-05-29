/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __ASM_UTILS_H__
#define __ASM_UTILS_H__

#include "autoconfig.h"

namespace utilities {

template <typename _int> _int add_with_carry(_int a, _int b, BYTE & c);
template <typename _int> bool bit_test(_int a, _int bit);

extern "C" {

extern void __cdecl init_xasmlib();

extern INT32 __cdecl add_with_carry32(UINT32 a, UINT32 b, BYTE & c);
extern INT64 __cdecl add_with_carry64(UINT64 a, UINT64 b, BYTE & c);

extern BYTE __cdecl incsl_8(BYTE w);
extern BYTE __cdecl incs_8(BYTE w);
extern WORD __cdecl incsl_16(WORD w);
extern WORD __cdecl incs_16(WORD w);
extern DWORD __cdecl incsl_32(DWORD w);
extern DWORD __cdecl incs_32(DWORD w);

extern BYTE BITTEST32(INT32 a, UINT32 bit); 
extern BYTE bittest64(INT64 a, UINT64 bit);

extern BYTE __cdecl vecadd(UINT64 * data1, UINT64 * data2, size_t len, BYTE initial_carry = 0);
extern BYTE __cdecl vecsub(UINT64 * data1, UINT64 * data2, size_t len, BYTE initial_carry = 0);
extern BYTE __cdecl vecadd_cipr(UINT64 * data1, UINT64 * data2, size_t len, BYTE initial_carry = 0);
extern void __cdecl vecshl(UINT64 * data, size_t shift, size_t len);
extern void __cdecl vecshr(UINT64 * data, size_t shift, size_t len);
extern void __cdecl vecand(UINT64 * data, UINT64 * data2, size_t len);
extern void __cdecl vecor(UINT64 * data, UINT64 * data2, size_t len);
extern void __cdecl vecxor(UINT64 * data, UINT64 * data2, size_t len);

extern void __cdecl fixending_64(UINT64 * data, DWORD value_bits, DWORD vwords, DWORD datalen);

extern void __cdecl vecor_elemwise(UINT64 * data, size_t len, DWORD bits, BYTE wordlen);
extern void __cdecl vecxor_elemwise(UINT64 * data, size_t len, DWORD bits, BYTE wordlen);
extern void __cdecl vecand_elemwise(UINT64 * data, size_t len, DWORD bits, BYTE wordlen);
extern void __cdecl vecadd_elemwise(UINT64 * data, size_t len, DWORD bits, BYTE wordlen);
extern void __cdecl vecsub_elemwise(UINT64 * data, size_t len, DWORD bits, BYTE wordlen);

extern UINT64 __cdecl extractword(UINT64 * source, BYTE bitofs, BYTE bits);
extern bool   __cdecl insertword(UINT64 source, UINT64 * target, BYTE bitofs, BYTE bits);

extern UINT64 __cdecl countbits (UINT64 * data, size_t len);
extern BYTE  __cdecl log2(UINT64 data);

// this is a stub on non-MMX code
extern void __cdecl do_emms();

#ifdef _HAVE_SIMD
/**
* MMX/SIMD versions of some of the above functions
*/
extern void __cdecl replace_if_mmx(UINT64 * data, UINT64 * data2, const UINT64 * mask, size_t len);
extern void __cdecl cmpxchg_8_mmx(UINT64 * data, UINT64 * data2, size_t len);
extern void __cdecl cmpxchg_16_mmx(UINT64 * data, UINT64 * data2, size_t len);
extern void __cdecl vecsatinc_8_mmx(UINT64 * data, size_t len);
extern void __cdecl vecsatinc_16_mmx(UINT64 * data, size_t len);
extern void __cdecl generate_match_mask_c8_v8_mmx(const UINT64 * string1, const UINT64 * string2, UINT64 * mask_out, size_t len);
extern void __cdecl generate_match_mask_c16_v16_mmx(const UINT64 * string1, const UINT64 * string2, UINT64 * mask_out, size_t len);

extern void __cdecl vecshl_mmx(UINT64 * data, size_t shift, size_t len);
#endif

};

template <> inline UINT32 add_with_carry(UINT32 a, UINT32 b, BYTE & c) {
	return add_with_carry32(a, b, c);
}

template <> inline UINT64 add_with_carry(UINT64 a, UINT64 b, BYTE & c) {
	return add_with_carry64(a, b, c);
}

template <> inline bool bit_test(UINT32 v, UINT32 bit) {
	return BITTEST32(v, bit) != 0;
}

template <> inline bool bit_test(UINT64 v, UINT64 bit) {
	return bittest64(v, bit) != 0;
}

};

#endif
