/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef RANGEPERFORMANCE_H_
#define RANGEPERFORMANCE_H_

#include <vector>

#include "bspcpp/tools/utilities.h"
#include "util/rs_container.h"

namespace rangesearching {

template<
	class _value,
	class _result,
    class _range,
	size_t _queries_per_element = 2	// number of queries carried out for each element in the range
> class RangeBenchmark {
public:
	double operator()(size_t num, _result init) {
		double t = 0;
		double tc;

		rs_container<_value> elements(num);

		/* fill array randomly. type _value should accept rand() integers for construction */
		for (size_t j = 0; j < num; ++j) {
			elements[j] = rand();
		}

		double t0 = utilities::time();
		_range rng(elements);
		double t1 = utilities::time();
		t += t1 - t0;
		tc = t1 - t0;
		
		/* test num queries */
		t0 = utilities::time();
		for(size_t j = 0; j < _queries_per_element*num; ++j) {
			_value l = rand();
			_value r = rand();
			_result v1(init), v2(init);
			
			if(r < l)
				std::swap(l, r);

			v1 = rng.query(l, r, v1);

			/* so the whole thing does not get optimized away */
			if(v1 != v2) {
				rand();
			}
		}
		t1 = utilities::time();
		t += t1 - t0;
#ifdef _VERBOSE
		std::cout << ">\t" << num << "\t" << tc << "\t" << t1 - t0 << std::endl;
#endif
		return t;
	}
};
};

#endif /*RANGEPERFORMANCE_H_*/
