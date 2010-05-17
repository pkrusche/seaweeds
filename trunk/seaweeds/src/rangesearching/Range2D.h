/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __RANGE2D_H__
#define __RANGE2D_H__

#include "config.h"

#include <iostream>
#include <math.h>
#include <algorithm>
#include <vector>

#include "bspcpp/tools/utilities.h"
#include "util/rs_container.h"
#include "xasmlib/functors.h"
#include "RangeList.h"
#include "BinTree.h"
#include "RangeTree.h"

namespace rangesearching {

/**
 * @brief 2D point storage class
 */
template<class _coord_t, int _size = 100> 
class Point2D : public std::pair<_coord_t,_coord_t> {
public:
	
	Point2D() {}
	
	/**
	 * @brief Initialize with coordinate pair
	 */
	Point2D(_coord_t x, _coord_t y) : std::pair<_coord_t,_coord_t>(x, y) {}

	/**
	 * @brief Initializing with a single integer gives a random point
	 */
	Point2D(int j) {
		if(j != 0) {
			this->first = (_coord_t )utilities::randnum(_size);
			this->second = (_coord_t )utilities::randnum(_size);
		} else {
			this->first = (_coord_t )0;
			this->second = (_coord_t )0;
		}
	}
};

/**
 * @brief Turn points into valid rectangle
 */
template <class _coord_t> 
void makerect(Point2D<_coord_t> & p1, 
  	          Point2D<_coord_t> & p2) {
	using namespace std;
	Point2D<_coord_t> p = p1;
	p1.first = min(p.first, p2.first);
	p1.second = min(p.second, p2.second);
	p2.first = max(p.first, p2.first);
	p2.second = max(p.second, p2.second);
}

template <class _T1, class _T2>
struct pair_first_less : public std::binary_function<std::pair<_T1, _T2>, std::pair<_T1, _T2>, bool> {
	bool operator()(const std::pair<_T1, _T2>& __x, const std::pair<_T1, _T2>& __y) const {
		return __x.first < __y.first;
	}
};

template <class _T1, class _T2>
struct pair_second_less : public std::binary_function<std::pair<_T1, _T2>, std::pair<_T1, _T2>, bool> {
	bool operator()(const std::pair<_T1, _T2>& __x, const std::pair<_T1, _T2>& __y) const {
		return __x.second < __y.second;
	}
};

/**
 *	@brief 2D range base class. 
 */
template <class _coord_t, 
		  class _add_fun = functors::count< Point2D<_coord_t> > 
		 > 
class Range2D {
public:
	typedef typename _add_fun::result _result_t;
	typedef rs_container < Point2D<_coord_t> > container_t;

	_add_fun  _plus; ///< public addition functor

	virtual _result_t query(Point2D<_coord_t> const & p1, Point2D<_coord_t> const & p2, _result_t & c0) = 0;

	bool inrange(Point2D<_coord_t> const & p1, Point2D<_coord_t> const & p2, Point2D<_coord_t> const & pt) {
		return ! (_less1(pt, p1) || _less1(p2, pt) || _less2(pt, p1) || _less2(p2, pt) );
	}

	virtual void dump() = 0;
private:
	pair_first_less<_coord_t, _coord_t> _less1;
	pair_second_less<_coord_t, _coord_t> _less2;
};

/**
 *	@brief 2D counting range. Stores points with given coordinate type and allows range counting. Uses nested lists.
 */
template <class _coord_t, class _add_fun = functors::count<Point2D<_coord_t> > > 
class Range2DLL : public Range2D<_coord_t, _add_fun> {
public:	
	typedef typename _add_fun::result _result_t;
	typedef rs_container < Point2D<_coord_t> > container_t;

	typedef NestedRangeList<
		Point2D<_coord_t>, 
		pair_first_less<_coord_t,_coord_t>, 
		_add_fun, 
		true,
		RangeList<
			Point2D<_coord_t>, 
			pair_second_less<_coord_t,_coord_t>, 
			_add_fun, 
			false
		> 
	> range_t;

	Range2DLL(container_t & container) : range(container) {}

	_result_t query(Point2D<_coord_t> const & p1, Point2D<_coord_t> const & p2, _result_t & c0) {
		return range.query(p1, p2, c0);
	}

	void dump() {
		range.dump();
	}

private:
	range_t range;
};

/**
 *	@brief 2D range. Stores points with given coordinate type and allows range counting. Uses Tree for first coordinate and sorted list for second.
 */
template <class _coord_t, class _add_fun = functors::count<Point2D<_coord_t> > > 
class Range2DTL : public Range2D<_coord_t, _add_fun> {
public:	
	typedef typename _add_fun::result _result_t;
	typedef rs_container < Point2D<_coord_t> > container_t;
	typedef RangeTree<
		Point2D<_coord_t>, 
		pair_first_less<_coord_t,_coord_t>, 
		_add_fun,
		BinTree<
			Point2D<_coord_t>, 
			pair_second_less<_coord_t,_coord_t>,  
			_add_fun,
			false
		> 
	> range_t;

	Range2DTL(container_t & container) : range(container) {}

	_result_t query(Point2D<_coord_t> const & p1, Point2D<_coord_t> const & p2, _result_t & c0) {
		return range.query(p1, p2, c0);
	}

	void dump() {
		range.dump();
	}

private:
	range_t range;
};

/**
 *	@brief 2D range. Stores points with given coordinate type and allows range counting. Uses Tree for first and second coordinates.
 */
template <class _coord_t, class _add_fun = functors::count<Point2D<_coord_t> > >
class Range2DTT : public Range2D<_coord_t, _add_fun> {
public:	
	typedef typename _add_fun::result _result_t;
	typedef rs_container < Point2D<_coord_t> > container_t;
	typedef RangeTree<
		Point2D<_coord_t>, 
		pair_first_less<_coord_t,_coord_t>, 
		_add_fun,
		RangeTree<
			Point2D<_coord_t>, 
			pair_second_less<_coord_t,_coord_t>, 
			_add_fun 
		>
	> range_t;

	Range2DTT(container_t & container) : range(container) {}

	_result_t query(Point2D<_coord_t> const & p1, Point2D<_coord_t> const & p2, _result_t & c0) {
		return range.query(p1, p2, c0);
	}

	void dump() {
		range.dump();
	}

private:
	range_t range;
};


// TL uses Range search for the first, and binary search for the second level
// for most applications, this should be the fastest method
#define Range2D Range2DTL

};

namespace std {
	template<class _coord_t> 
	std::ostream & operator<< (std::ostream & o, rs_container< rangesearching::Point2D<_coord_t> > const & c) {
		for(typename rs_container< rangesearching::Point2D<_coord_t> >::const_iterator it = c.begin(); it != c.end(); ++it) {
			o << *it << ",";
		}
		return o;
	}

	template<class _coord_t> 
	bool operator== (rs_container< rangesearching::Point2D<_coord_t> > const & c1, rs_container< rangesearching::Point2D<_coord_t> > const & c2) {
		bool t = true;
		for(typename rs_container< rangesearching::Point2D<_coord_t> >::const_iterator it = c1.begin(); it != c1.end(); ++it) {
			if(std::find(c2.begin(), c2.end(), *it) == c2.end() ) {
				t = false;
				break;
			}
		}
		return t;
	}

};


#endif
