/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef RANGEPERFORMANCE_H_
#define RANGEPERFORMANCE_H_

#include <vector>

#include "bsp.h"
#include "util/rs_container.h"
#include "Range2D.h"

namespace rangesearching {

template<
	class _value,
	class _result,
    class _range,
	size_t _queries_per_element = 2	// number of queries carried out for each element in the range
> class RangeBenchmark2D {
public:
	double operator()(size_t num, _result init) {
		double t = 0;
		double tc;

		rs_container<_value> elements(num);

		/* fill array randomly. type _value should accept rand() integers for construction */
		for (size_t j = 0; j < num; ++j) {
			elements[j] = rand();
		}

		double t0 = bsp_time();
		_range rng(elements);
		double t1 = bsp_time();
		t += t1 - t0;
		tc = t1 - t0;
		
		/* test num queries */
		t0 = bsp_time();
		for(size_t j = 0; j < _queries_per_element*num; ++j) {
			_value l(rand());
			_value r(rand());
			_result v1(init), v2(init);
			
			makerect(l, r);

			v1 = rng.query(l, r, v1);

			/* so the whole thing does not get optimized away */
			if(v1 != v2) {
				rand();
			}
		}
		t1 = bsp_time();
		t += t1 - t0;
#ifdef _VERBOSE
		std::cout << ">\t" << num << "\t" << tc << "\t" << t1 - t0 << std::endl;
#endif
		return t;
	}
};
};

#endif /*RANGEPERFORMANCE_H_*/
