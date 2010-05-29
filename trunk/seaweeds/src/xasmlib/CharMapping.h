/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __CHARMAPPING_H__
#define __CHARMAPPING_H__

#include "autoconfig.h"

#include <vector>

#include "IntegerVector.h"

namespace utilities {

template<int _bpc, UINT64 _mapval = 1, bool _invert = true>
class CharMapping : public std::vector< BitString > {
public:

	CharMapping(IntegerVector<_bpc> const & _str) {
		UINT64 alphasize= (1 << _bpc);
		size_t xlen= _str.size();

		BitString _template;
		resize((size_t)alphasize, _template);

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
};
};

			
#endif

