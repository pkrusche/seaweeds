/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __INTEGERVECTOR_VECTORCLASS_INL__
#define __INTEGERVECTOR_VECTORCLASS_INL__

#include <vectorclass.h>

namespace utilities {

	/** 8-bit SSE/AVX specialisation using vectorclass */
	template <> inline void IntegerVector<8>::cmpxchg_masked(IntegerVector<8> & t, IntegerVector<8> & ms) {
		ASSERT(vword_len == t.vword_len && vword_len == ms.vword_len);
		size_t len_p = vword_len >> 4;
		size_t len_r = vword_len & (0xf);
		Vec16uc v1, v2, vm, vr, zero = constant4i<0, 0, 0, 0>();

		BYTE * d1 = (BYTE*)content.data;
		BYTE * d2 = (BYTE*)t.content.data;
		BYTE * m = (BYTE*)ms.content.data;

		for (int i = 0; i < len_p; ++i)	{
			v1.load_a(d1);
			v2.load_a(d2);
			vm.load_a(m);

			// vm is 0 or 255 
			vm = vm != zero;
			vr = selectb(  vm, v2, min(v1, v2));
			vr.store(d1);
			vr = selectb(  vm, v1, max(v1, v2));
			vr.store(d2);
			
			d1+= 16;
			d2+= 16;
			m+= 16;
		}
		if (len_r > 0)	{
			v1.load_partial((int)len_r, d1);
			v2.load_partial((int)len_r, d2);
			vm.load_partial((int)len_r, m);

			vm = vm != zero;

			vr = selectb( vm, v2, min(v1, v2));
			vr.store_partial((int)len_r, d1);

			vr = selectb( vm, v1, max(v1, v2));
			vr.store_partial((int)len_r, d2);
		}
	}
};


#endif
