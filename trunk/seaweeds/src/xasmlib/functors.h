/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __FUNCTORS_H__
#define __FUNCTORS_H__

#include <function.h>

#include "../util/rs_container.h"
#include "../xasmlib/xasmlib.h"

namespace functors {

/**
 * @brief counting functor
 *
 * <summary>
 * This functor adds one to the first integer parameter and igores
 * the second one of type val. Useful for counting objects.
 * </summary>
 */
template<class val>
struct count : public fcpp::Fun2Impl<size_t, val, size_t> {
	typedef size_t result;

    size_t operator() (const size_t & count, const val &) const {
        return count + 1;
    }

    size_t operator() (size_t count1, size_t count2) const {
        return count1 + count2;
    }

};


/**
 * @brief counting and reporting functor
 *
 * <summary>
 * Records every value that is passed in a list. This is not a
 * functor, as the list parameter is not a const &
 * </summary>
 */
template<class value_type, class list = rs_container<value_type> >
struct report {
	typedef list result;
    list & operator() (list & l, const value_type & v) const {
        l.insert(l.end(), v);
        return l;
    }

	list & operator() (list & l, const list & v) const {
        l.insert(l.end(), v.begin(), v.end());
        return l;
    }
};

// gcc has issues with transform et al. We use these instead.

template<class _t, template <class t> class _binary_operator > void elementwise_operation(_t * ptr, _t param, size_t len) {
	static _binary_operator<_t> op;
	for(size_t i = 0; i < len; ++i) {
		*ptr = op(*ptr, param);
		ptr++;
	}
};

template<class _t, template <class t> class _unary_operator > void elementwise_operation_1(_t * ptr, size_t len) {
	static _unary_operator<_t> op;
	for(size_t i = 0; i < len; ++i) {
		*ptr = op(*ptr);
		ptr++;
	}
};

template <class _t> class _add {
public:
	_t operator() (_t x1, _t x2) const {
		return x1+x2;
	}
};

template <class _t> class _sub {
public:
	_t operator() (_t x1, _t x2) const {
		return x1-x2;
	}
};

template <class _t> class _and {
public:
	_t operator() (_t x1, _t x2) const {
		return x1&x2;
	}
};

template <class _t> class _or {
public:
	_t operator() (_t x1, _t x2) const {
		return x1|x2;
	}
};

template <class _t> class _xor {
public:
	_t operator() (_t x1, _t x2) const {
		return x1^x2;
	}
};

template <class _t> class _bt {
public:
	_t operator() (_t value, _t bit) const {
		const int _t_bits = sizeof(_t)*8;
		const _t msb = (_t)((static_cast<UINT64>(1)) << (_t_bits - 1));
		const _t lsbs = msb - 1;
		return (_t)(((( (value & ((static_cast<UINT64>(1)) << bit) ) >> bit) | msb) - 1) ^ lsbs);
	}
};

template <class _t> class _adds {
public:
	_t operator() (_t v1, _t v2) const {
		const int _t_bits = sizeof(_t)*8;
		const _t msb = (_t)((static_cast<UINT64>(1)) << (_t_bits - 1));
		const _t lsbs = msb - 1;

		// these test if the top bits are set
		_t o1 = v1;
		_t o2 = v2;
		_t tmp1 = 0 - ((v1) >> (_t_bits-1));
		_t tmp2 = 0 - ((v2) >> (_t_bits-1));

		// add lsbs first
		v1&= lsbs;
		v2&= lsbs;
		v1+= v2;

		// check if addition caused overflow
		v2 = 0 - ((v1 & msb) >> (_t_bits-1));

		// set top bit if necessary
		v1|= (o1 | o2 | v2 )& msb;

		// more than 2 top bits set -> saturate
		v1|= tmp1&tmp2;
		v1|= v2&tmp1;
		v1|= v2&tmp2;

		return v1;
	}
};

template <class _t> class _incs {
public:
	_t operator() (_t v1 ) const {
		_adds<_t> plus;
		return plus(v1, 1);
	}
};

template <>  
BYTE _incs<BYTE>::operator() (BYTE v1 ) const {
	return utilities::incs_8(v1);
}

template <>  
WORD _incs<WORD>::operator() (WORD v1 ) const {
	return utilities::incs_16(v1);
}

template <>  
DWORD _incs<DWORD>::operator() (DWORD v1 ) const {
	return utilities::incs_32(v1);
}

};

#endif
