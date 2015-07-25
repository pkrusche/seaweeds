/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __RANGELIST_H__
#define __RANGELIST_H__

#include <algorithm>

#include "util/rs_container.h"
#include "IRange.h"

namespace rangesearching {

/**
 * <brief>PrecomputeSum class.</brief>
 * <summary>
 * Precomputes and stores the sum over the given interval.
 * </summary>
 */
template <
	class _value_type,
	class _add_fun
> class PrecomputeSum {
public:
	typedef typename _add_fun::result _query_type;
	typedef typename rs_container<_value_type>::iterator iterator;
	_add_fun _plus;

	PrecomputeSum(rs_container<_value_type> & data) : result(0) {
		precomp(data.begin(), data.end());
	}

	PrecomputeSum(iterator _b, iterator _e) : result(0) {
		precomp(_b, _e);	
	}

	_query_type & query(
			const _value_type & p1, 
			const _value_type & p2, 
			_query_type & _res
	) {
		_res = _plus(_res, (_query_type)result);
		return _res;
	}

private:
	void precomp(iterator b, iterator e) {
		while (b != e) {
			result = _plus(result, *b);
			++b;
		}
	}

	_query_type result;
};

/**
 * <brief>RangeList class.</brief>
 * <summary>
 * Implementation of linear time range search on a list. No preprocessing time, no additional storage, and O(n) search cost.
 * </summary>
 */
template <
	class _value_type,
	class _comparison_fun,
	class _add_fun,
	bool copy
> class RangeList : public IRange<_value_type, rs_container<_value_type>, _comparison_fun, _add_fun, copy> {
public:
	typedef typename _add_fun::result _query_type;
	typedef IRange<_value_type, rs_container<_value_type>, _comparison_fun, _add_fun, copy> _base_type; 
	typedef typename rs_container<_value_type>::iterator iterator;

	RangeList(rs_container<_value_type> & data) : 
		_base_type(data.begin(), data.end()) {
	}

	RangeList(iterator _b, iterator _e) : 
		_base_type(_b, _e) {
	}

	_query_type & query(
			const _value_type & p1, 
			const _value_type & p2, 
			_query_type & result
	) {
		static _comparison_fun _less;
		static _add_fun _plus;
		iterator it = _base_type::begin();
		iterator iend = _base_type::end(); 

		while (it != iend) {
			if(!_less(*it, p1) && !_less(p2, *it)) {
				result = _plus(result, *it);
			}
			++it;
		}

		return result;
	}
};

/**
 * <brief>Nested RangeList class.</brief>
 * <summary>
 * Implementation of linear time range search on a list. No preprocessing time, no additional storage, and O(n) search cost.
 * For each element found, another query will be carried out.
 * </summary>
 */
template <
	class _value_type,
	class _comparison_fun,
	class _add_fun,
	bool copy, 
	class _nested_type
> class NestedRangeList : 
	public IRange<_value_type, rs_container<_value_type>, _comparison_fun, _add_fun, copy> {
public:
	typedef typename _add_fun::result _query_type;
	typedef IRange<_value_type, rs_container<_value_type>, _comparison_fun, _add_fun, copy> _my_base;
	typedef typename rs_container<_value_type>::iterator iterator;

	NestedRangeList(rs_container<_value_type> & data) : 
		_my_base(data.begin(), data.end()) {
	}

	NestedRangeList(iterator _b, iterator _e) : 
		_my_base(_b, _e) {
	}

	_query_type & query(
			const _value_type & p1, 
			const _value_type & p2, 
			_query_type & result
	) {
		static _comparison_fun _less;
		iterator it = _my_base::begin();
		iterator iend = _my_base::end(); 
		rs_container<_value_type> temp;

		while (it != iend) {
			if(!_less(*it, p1) && !_less(p2, *it)) {
				temp.insert(temp.end(), *it);
			}
			++it;
		}
		_nested_type nest(temp);
		return nest.query(p1, p2, result);
	}
};


};


#endif /*RANGELIST_H_*/

