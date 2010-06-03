/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __IRANGE_H__
#define __IRANGE_H__

#include <iostream>
#include <string>

#include "util/TypeList.h"

namespace rangesearching {

/**
 * @brief Range interface.
 * 
 * <summary>
 * Defines an interface for query ranges. Such classes must 
 * implement a query function that, given two values returns
 * a query-reply.
 * </summary>
 */
template <
	class _value_type,
	class _container_type,
	class _comparison_fun,
	class _add_fun,
	bool copy
> class IRange {
public:
	typedef IRange<_value_type, _container_type, _comparison_fun, _add_fun, copy> _my_type;
	typedef typename _add_fun::result _query_type;

	_add_fun _plus; ///< addition operator
	_comparison_fun _less; ///< less operator

	virtual void dump(int level = 0) {
		std::string s;
		for(int l = 0; l < level; ++l) {
			s+= "\t";
		}
		std::cout << s.c_str() << "[";
		
		for(_iterator_type it = begin(); it != end(); ++it) {
			std::cout << *it << ",";
		}
		std::cout << std::endl;
	}

protected:
	typedef typename _container_type::iterator _iterator_type;

	struct CopyInitializer {
		void operator()(_my_type & rthis, _container_type ** container, _iterator_type & first, _iterator_type & last) {
			*container = new _container_type(first, last);
			rthis.first = (*container)->begin();
			rthis.last = (*container)->end();
		}
	};

	struct LinkInitializer {
		void operator()(_my_type & rthis, _container_type ** , _iterator_type & _begin, _iterator_type & _end) {
			rthis.first = _begin;
			rthis.last = _end;
		}
	};

	IRange(_iterator_type _first, _iterator_type _last) : first(_first), last (_last) {
		typename utilities::Select<copy, CopyInitializer, LinkInitializer>::Result i;
		i(*this, &container, first, last);
	}

	virtual ~IRange() {
		if(copy) {
			delete container;
		}
	}

	/**
	 * Query all entries between p1 and p2, return a query result.
	 * 
	 * @param p1 "left" end of the range
	 * @param p2 "right" end of the range
	 * @param left initializer for element accumulation in the range (a'la foldl but not quite since we allow modification to cater for lists)
	 * @return the query result
	 */
	virtual _query_type & query(
		const _value_type & p1,
		const _value_type & p2,
		_query_type & left
	) = 0;

	virtual _iterator_type begin() const {
		return first;
	}

	virtual _iterator_type end() const {
		return last;
	}

	static bool leq(const _value_type & p1, const _value_type & p2) {
		return !l(p2, p1);
	}

private:
	IRange() {}

	_iterator_type first;	///< begin iterator
	_iterator_type last;	///< end iterator

	_container_type * container; ///< value container if copy template argument is true
};

};

#endif
