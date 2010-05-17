/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef _MACHINEWORD_FIXED_H
#define	_MACHINEWORD_FIXED_H

/*
 * This class deals with very long bitstrings which are subdivided into 
 * words. It is intended to provide a relatively complete and efficient 
 * implementation of useful routines in bit/word vector arithmetic.
 */

#include <algorithm>

#include "IntegerVector.h"
#include "functors.h"

namespace utilities {
	// specialize for BYTEs, WORDs, DWORDs and UINT64s.
	
	// TODO: a nifty metaprogram which does this automatically would be nice
	template<> IntegerVector<8> & IntegerVector<8>::operator+= (UINT64 j) {
		functors::elementwise_operation<BYTE, functors::_add> ((BYTE *)content.data, (BYTE)j, vword_len);
		return *this;
	}

	template<> IntegerVector<8> & IntegerVector<8>::operator-= (UINT64 j) {
		functors::elementwise_operation<BYTE, functors::_sub> ((BYTE *)content.data, (BYTE)j, vword_len);
		return *this;
	}

	template<> IntegerVector<8> & IntegerVector<8>::operator&= (UINT64 j) {
		functors::elementwise_operation<BYTE, functors::_and> ((BYTE *)content.data, (BYTE)j, vword_len);
		return *this;
	}

	template<> IntegerVector<8> & IntegerVector<8>::operator|= (UINT64 j) {
		functors::elementwise_operation<BYTE, functors::_or> ((BYTE *)content.data, (BYTE)j, vword_len);
		return *this;
	}

	template<> IntegerVector<8> & IntegerVector<8>::operator^= (UINT64 j) {
		functors::elementwise_operation<BYTE, functors::_xor> ((BYTE *)content.data, (BYTE)j, vword_len);
		return *this;
	}

	template <> void IntegerVector<8>::bittest(int c) {
		functors::elementwise_operation<BYTE, functors::_bt> ((BYTE *)content.data, (BYTE)c, vword_len);
	}

	template<> IntegerVector<16> & IntegerVector<16>::operator+= (UINT64 j) {
		functors::elementwise_operation<WORD, functors::_add> ((WORD *)content.data, (WORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<16> & IntegerVector<16>::operator-= (UINT64 j) {
		functors::elementwise_operation<WORD, functors::_sub> ((WORD *)content.data, (WORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<16> & IntegerVector<16>::operator&= (UINT64 j) {
		functors::elementwise_operation<WORD, functors::_and> ((WORD *)content.data, (WORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<16> & IntegerVector<16>::operator|= (UINT64 j) {
		functors::elementwise_operation<WORD, functors::_or> ((WORD *)content.data, (WORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<16> & IntegerVector<16>::operator^= (UINT64 j) {
		functors::elementwise_operation<WORD, functors::_xor> ((WORD *)content.data, (WORD)j, vword_len);
		return *this;
	}

	template <> void IntegerVector<16>::bittest(int c) {
		functors::elementwise_operation<WORD, functors::_bt> ((WORD *)content.data, (WORD)c, vword_len);
	}


	template<> IntegerVector<32> & IntegerVector<32>::operator+= (UINT64 j) {
		functors::elementwise_operation<DWORD, functors::_add> ((DWORD *)content.data, (DWORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<32> & IntegerVector<32>::operator-= (UINT64 j) {
		functors::elementwise_operation<DWORD, functors::_sub> ((DWORD *)content.data, (DWORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<32> & IntegerVector<32>::operator&= (UINT64 j) {
		functors::elementwise_operation<DWORD, functors::_and> ((DWORD *)content.data, (DWORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<32> & IntegerVector<32>::operator|= (UINT64 j) {
		functors::elementwise_operation<DWORD, functors::_or> ((DWORD *)content.data, (DWORD)j, vword_len);
		return *this;
	}

	template<> IntegerVector<32> & IntegerVector<32>::operator^= (UINT64 j) {
		functors::elementwise_operation<DWORD, functors::_xor> ((DWORD *)content.data, (DWORD)j, vword_len);
		return *this;
	}

	template <> void IntegerVector<32>::bittest(int c) {
		functors::elementwise_operation<DWORD, functors::_bt> ((DWORD *)content.data, (DWORD)c, vword_len);
	}

	template <> void IntegerVector<32>::saturated_inc() {
		functors::elementwise_operation_1<DWORD, functors::_incs> ((DWORD *)content.data, vword_len);
	}

	template <> void IntegerVector<8>::put(size_t pos, int value) {
		((BYTE*)content.data)[pos] = (BYTE)value;
	}

	template <> int IntegerVector<8>::get(size_t pos) const {
		return ((const BYTE*)content.data)[pos];
	}

	template <> void IntegerVector<8>::fixending() {
		memset(((BYTE*)content.data) + vword_len, 0, 8*content.size-vword_len);
	}

	template <> void IntegerVector<16>::fixending() {
		memset(((WORD*)content.data) + vword_len, 0, 8*content.size-(2*vword_len));
	}

	template <> void IntegerVector<32>::fixending() {
		memset(((WORD*)content.data) + vword_len, 0, 8*content.size-(4*vword_len));
	}


	/** SIMD accelerated specialisations. */
#ifdef _HAVE_SIMD
	template <> void IntegerVector<8>::saturated_inc() {
		vecsatinc_8_mmx(content.data, content.size);
	}

	template<> void IntegerVector<8>::generate_match_mask
		( const IntegerVector<8> & s1, const IntegerVector<8> & s2 ) {
			ASSERT(s1.size() == s2.size() && s1.size() == size());
			generate_match_mask_c8_v8_mmx(s1.content.data, s2.content.data, content.data, content.size);
//			fixending();
	}

	template<> void IntegerVector<8>::cmpxchg(IntegerVector<8> & t) {
		using namespace std;
		ASSERT(vword_len == t.vword_len);
		cmpxchg_8_mmx(content.data, t.content.data, content.size);
	}

	template <> void IntegerVector<16>::saturated_inc() {
		vecsatinc_16_mmx(content.data, content.size);
	}

	template<> void IntegerVector<16>::generate_match_mask
		( const IntegerVector<16> & s1, const IntegerVector<16> & s2 ) {
			ASSERT(s1.size() == s2.size() && s1.size() == size());
			utilities::generate_match_mask_c16_v16_mmx(s1.content.data, s2.content.data, content.data, content.size);
//			fixending();
	}

	/**
	* compare-exchange: exchanges elements (*this)[i] and t[i] iff. 
	* (*this)[i] > t[i]
	*/
	template<> void IntegerVector<16>::cmpxchg(IntegerVector<16> & t) {
		using namespace std;
		ASSERT(vword_len == t.vword_len);
		cmpxchg_16_mmx(content.data, t.content.data, content.size);
	}
#endif
};

#endif

