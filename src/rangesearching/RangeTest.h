/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef RANGETEST_H_
#define RANGETEST_H_

#include <stdlib.h>
#include <vector>

#include "util/rs_container.h"
#include "IRange.h"

namespace rangesearching {

template<
	class _value,
	class _result,
    class _range
> class RangeTest {
public:
	bool operator()(size_t num, _result init) {
		rs_container<_value> elements(num);

		/* fill array randomly. type _value should accept rand() integers for construction */
		for (size_t j = 0; j < num; ++j) {
			elements[j] = rand();
		}

		_range rng(elements);

		bool range_printed = false;

		/* test num queries */
		bool result = true;
		for(size_t j = 0; j < num; ++j) {
			_value l = rand();
			_value r = rand();
			_result v1(init), v2(init);

			if(l > r) {
				std::swap(l, r);
			}
			v1 = rng.query(l, r, v1);

			/* test by linear search */
			for(size_t k = 0; k < num; ++k) {
				if(!rng._less(elements[k], l) && !rng._less(r,elements[k])) {
					v2 = rng._plus(v2, elements[k]);
				}
			}

			if(v1 != v2) {
				if(!range_printed) {
					std::cout << "All values: ";
					for(size_t k = 0; k < num; ++k) {
						std::cout << elements[k] << ", ";
					}					
					std::cout << std::endl;
					range_printed = true;
				}
				std::cout << " : Mismatch, tested range [" << l << ", " << r << "]: " << v1 << " != actual " << v2 << std::endl;
				result = false;
			}
		}
		return result;
	}
};

};

#endif /*RANGETEST_H_*/
