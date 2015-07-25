/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef __CHARMAPPING_H__
#define __CHARMAPPING_H__

#include "autoconfig.h"

#include <vector>

#include "IntegerVector.h"

namespace utilities {

template<int _bpc, UINT64 _mapval = 1, bool _invert = true>
class CharMapping {
public:

	enum {
		alphasize = (1 << _bpc)
	};

	CharMapping(IntegerVector<_bpc> const & _str) 
	{
		_x = _str;
		size_t xlen= _str.size();

		for (size_t k= 0; k < alphasize; ++k) {
			(*this)[k].resize(xlen);
			(*this)[k].zero();
		}
		for (size_t j= 0;j < xlen; ++j) {
			(*this)[(size_t)_str[j]][j] = _mapval; 
		}
		if(_invert) {
			for (size_t k= 0; k < alphasize; ++k) {
				(*this)[k]=~(*this)[k];
			}
		}
	}

	BitString & operator [] (size_t p) {
		ASSERT(p < alphasize);
		return data[p];
	}

	size_t size() const {
		return alphasize;
	}

	IntegerVector<_bpc> const & string() {
		return _x;
	}

private:
	IntegerVector<_bpc> _x;
	BitString data [alphasize];
};
};

			
#endif

