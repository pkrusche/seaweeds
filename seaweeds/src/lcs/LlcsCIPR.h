/***************************************************************************
*   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
*   peter@dcs.warwick.ac.uk                                               *
***************************************************************************/

/**
 * The sequential algorithm is introduced in:
 * Crochemore, M., Iliopoulos, C. S., Pinzon, Y. J.,
 * and Reid, J. F. 2001.
 * A fast and practical bit-vector algorithm for the longest common
 * subsequence problem.
 * Inf. Process. Lett. 80, 6 (Dec. 2001), 279-285.
 * DOI= http://dx.doi.org/10.1016/S0020-0190(01)00182-X
 */

#ifndef __LCSCIPR_H__
#define __LCSCIPR_H__

#include <math.h>
#include <vector>

#include "xasmlib/IntegerVector.h"
#include "xasmlib/CharMapping.h"

namespace lcs {

	std::ostream & operator<<(std::ostream & o, std::vector<UINT64> const & v) {
		for(size_t j = v.size()-1; j > 0; --j) {
			o << v[j];
		}
		o << v[0];
		return o;
	}

template <size_t _bpc>
class LlcsCIPR {
public:

	enum { bits_per_char = _bpc };
	typedef utilities::IntegerVector<_bpc> string;

#ifdef LCS_VERIFY
	INT32 count_bits(INT32 i) {
		i = i - ((i >> 1) & 0x55555555);
		i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
		return ((i + (i >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
	}
#endif

	size_t operator()(
		string const & _x, string const & _y,
		utilities::BitString *_r = NULL,
		utilities::BitString *_c = NULL
	) {
		size_t xlen= _x.size();
		utilities::CharMapping<_bpc, 1, false> gM(_x);
		/* we use a fixed mapping which is precomputed */
		return (*this)(xlen, gM, _y, _r, _c);
	}

	/*
	* Compute length of longest common subsequence sequentially
	* using CIPR bit parallel algorithm
	* */
	size_t operator()(
		size_t xlen,
		utilities::CharMapping<_bpc, 1, false> & gM,
		utilities::IntegerVector<_bpc> const & _y,
		utilities::BitString *_r = NULL,
		utilities::BitString *_c = NULL
		) {
		using namespace std;
		using namespace utilities;

		/* extract to local variables */
		size_t ylen= _y.size();
		size_t lbits= 8 * sizeof(UINT64);
		size_t rlen= xlen % lbits;
		size_t lxlen= xlen / lbits;

		bool alloc_r = false;
		if (_r == NULL) {
			_r = new BitString(xlen);
			_r->one();
			alloc_r = true;
		}

		bool alloc_c = false;
		if (_c == NULL) {
			_c = new BitString(ylen);
			_c->zero();
			alloc_c = true;
		}

		for (size_t j= 0; j < ylen; ++j) {
			UINT64 _chr = _y[j];
			UINT64 _carry = (*_c)[j];
#ifdef LCS_VERIFY
			BitString rb = *_r;
#endif
			_r->add_cipr(gM[(size_t)_chr], (BYTE) _carry);

#ifdef LCS_VERIFY
			INT32 L, M, carrybyte = 0;
			int bits = 0;
			for (size_t xx = 0; xx < rb.size(); ++xx) {
				ASSERT(gM.size() < _chr);
				L = rb[xx];
				M = gM[(size_t)_chr][xx];
				L = L+(L&M) | (L&~M) + carrybyte;
				carrybyte = ((L & 0x2) != 0) ? 1 : 0;
				L&= 0x1;
				bits+= count_bits(L);
				if(L != _r->get(xx)) {
					cout << "FAIL: CIPR " << j << " L = " << L << " r = "<< _r[xx] << endl;
				}
			}
			if(_r->size() - bits != _r->count_zeros()) {
				cout << "FAIL: bitcount " << bits << " != " << _r->count_zeros() << " (" << _r->count_bits() << ")" << endl;
			}
#endif
			(*_c)[j] = _r->carry();
		}

		size_t count = _r->count_zeros();
#ifdef LCS_VERIFY
		lcs::Llcs<string> l;
		ASSERT(l(_x, _y) == count);
#endif
		if (alloc_c) {
			delete _c;
		}

		if (alloc_r) {
			delete _r;
		}

		return count;
	}

};

};

#endif
