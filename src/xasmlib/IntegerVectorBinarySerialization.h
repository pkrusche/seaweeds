/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __IntegerVectorBinarySerialization_H__
#define __IntegerVectorBinarySerialization_H__

/************************************************************************/
/* Make IntegerVector objects BSP binary-serializable                   */
/*                                                                      */
/************************************************************************/

#include "bsp_cpp/Shared/SharedVariable.h"

#include <boost/preprocessor/repeat.hpp>

namespace bsp {



#define _bpc 8
	template <> 
	inline size_t SharedSerializable< utilities::IntegerVector<_bpc> >::serialized_size() {
		utilities::IntegerVector<_bpc> * thiz = (utilities::IntegerVector<_bpc>*)valadr;
		return  (thiz->datavector().exact_size() << 3) + sizeof(UINT64) * 3;
	}

	template <> 
	inline void SharedSerializable<utilities::IntegerVector<_bpc> >::serialize (void * target, size_t nbytes) {
		ASSERT (this->serialized_size() >= nbytes);
		utilities::IntegerVector<_bpc> * thiz = (utilities::IntegerVector<_bpc>*)valadr;
		
		UINT64 * t = (UINT64*) target;
		*t++ = thiz->datavector().exact_size();
		*t++ = thiz->size();
		*t++ = thiz->carry() ? 0 : 1;
		A_memcpy (t, thiz->datavector().exact_data(0), thiz->datavector().exact_size() << 3);
	}

	template <> 
	inline void SharedSerializable<utilities::IntegerVector<_bpc> >::deserialize(void * source, size_t nbytes) {
		ASSERT ( nbytes >= sizeof(UINT64) * 3 );
		utilities::IntegerVector<_bpc> * thiz = (utilities::IntegerVector<_bpc>*)valadr;
		UINT64 * t = (UINT64*) source;
		UINT64 vlen = *t++;
		UINT64 wlen = *t++;
		UINT64 c = *t++;
		ASSERT ( nbytes >= sizeof(UINT64) * vlen );
		thiz->resize(wlen);
		ASSERT ( thiz->datavector().exact_size() >= vlen );
		A_memcpy ( thiz->datavector().exact_data(0), t, thiz->datavector().exact_size() << 3 );
		thiz->fixending();
		if (c) {
			thiz->stc();
		}
	}
#undef _bpc
};

#endif // __IntegerVectorBinarySerialization_H__
