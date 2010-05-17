/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __BINTREE_H__
#define __BINTREE_H__

#include <loki/TypeManip.h>
#include <loki/NullType.h>

#include "bspcpp/Permutation.h"
#include "bspcpp/tools/utilities.h"

#include "util/rs_container.h"
#include "IRange.h"

namespace rangesearching {

/**
 * <brief>BinTree class.</brief}
 * <summary>
 * Essentially an implementation of 1-D binary search on a list.
 * </summary>
 */
template <
	class _value_type,
	class _comparison_fun,
	class _add_fun,
	bool copy = false,
	class _nesting = Loki::NullType
> class BinTree : public IRange<_value_type, 
								rs_container<_value_type>,
								_comparison_fun, 
								_add_fun, 
								copy
								> {
public:
	typedef typename _add_fun::result _query_type;
	typedef IRange<_value_type, rs_container<_value_type>, _comparison_fun, _add_fun,copy> _base_type;
	
	typedef BinTree <_value_type,
					 _comparison_fun,
					 _add_fun,
					 copy,
					 _nesting
					> my_type;
	/**
	 * @brief shortcut for container type
	 */
	typedef rs_container<_value_type>  container_type;

	/**
	 * @brief when initializing, we want to accept proper container iterators and not permutation iterators
	 */
	typedef typename rs_container<_value_type> :: iterator iterator;

	/**
	 * @brief iterators of this type will proceed through the sequence in sorted order
	 */
	typedef typename utilities::Permutation< container_type > :: iterator sorted_iterator;

	typedef typename utilities::PermutationComparisonFun<_value_type, 
					iterator, _comparison_fun> _less_type;

	struct DirectQuery {
		DirectQuery() {}

		_query_type & operator() (
			const _value_type & p1, 
			const _value_type & p2, 
			sorted_iterator start, 
			sorted_iterator end, 
			_query_type & result) {
			_add_fun _plus;
			while (start != end) {
				result = _plus(result, *start);
				++start;
			}

			return result;
		}
	};

	struct DirectCountingQuery {
		DirectCountingQuery() {}

		_query_type & operator() (
			const _value_type & p1, 
			const _value_type & p2, 
			sorted_iterator start, 
			sorted_iterator end, 
			_query_type & result
		) {
			static functors::count<_value_type>  _plus;

			result = _plus(result, (size_t)std::distance(start, end));

			return result;
		}
	};


	struct NestedQuery {
		NestedQuery() {}

		_query_type & operator() (
			const _value_type & p1, 
			const _value_type & p2, 
			sorted_iterator start, 
			sorted_iterator end, 
			_query_type & result
		) {
			if (start != end) {
				// DANG! This does not work. if we dereference start and 
				// end, there is no safe way of telling which one came
				// first in the original sequence.
				// TODO: Fix nesting for BinTree
				_nesting nestquery(start, end);
				result = nestquery.query(p1, p2, result);
			}
			return result;
		}
	};

	typedef typename Loki::Select<
		Loki::IsSameType<_nesting, Loki::NullType>::value,
		typename Loki::Select<		
			Loki::IsSameType<_add_fun, typename functors::count<_value_type> >::value,
			DirectCountingQuery,
			DirectQuery
		>::Result,
		NestedQuery>::Result QueryMethod;
	
	BinTree(rs_container<_value_type> & data) : 
		_base_type(data.begin(), data.end()), sorting_permutation (_base_type::begin(), _base_type::end())
	{
		_less_type _lesst;
		std::sort(
				sorting_permutation.begin(), 
				sorting_permutation.end(), 
				_lesst);
	}

	BinTree(iterator _b, iterator _e) : 
		_base_type(_b, _e), sorting_permutation (_base_type::begin(), _base_type::end())
	{	
		_less_type _lesst;
		std::sort(
				sorting_permutation.begin(), 
				sorting_permutation.end(), 
				_lesst);
	}
	
	_query_type & query(
			const _value_type & p1, 
			const _value_type & p2, 
			_query_type & result
	) {
		using namespace std;
		QueryMethod query_method;
		_less_type _lesst;

		sorted_iterator start_it = lower_bound(
				sorting_permutation.begin(), 
				sorting_permutation.end(), p1, 
				_lesst);
		sorted_iterator end_it = upper_bound(
				sorting_permutation.begin(), 
				sorting_permutation.end(), p2, 
				_lesst);

		return query_method(p1, p2, start_it, end_it, result);
	}

private:
	utilities::Permutation< container_type > sorting_permutation;
};

};


#endif

