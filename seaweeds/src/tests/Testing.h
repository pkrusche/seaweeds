/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __TESTING_H__
#define __TESTING_H__

#include <math.h>
#include <vector>
#include <list>
#include <map>

#include "util/Typelist.h"

#include "bspcpp/tools/utilities.h"

namespace tuning {

template <typename _result, class _parameter, class _tlist> class MultipleTests;

template<class _result, class _parameter, class _tlist> class TestIterator {
public:
	/**
     * @brief Time for all num parameters from add, increasing by inc
     */
	void operator() (int add, int NUM, int inc) {
		using namespace std;
		for(int num = 0; num < NUM; ++num) {
			std::list<_result> data;
			size_t actual = num*inc + add;
			_parameter p(actual);
			MultipleTests<_result, _parameter, _tlist>::test(p, data);
			_result initial = *(data.begin());
			bool all_match = true;
			for(typename std::list<_result>::iterator it = data.begin();
				it != data.end();
				it++) {
				if(*it != initial) {
					all_match = false;
					break;
				}
			}

			if(!all_match) {
				cout << "n: " << actual << "\t";
				for(typename std::list<_result>::iterator it = data.begin();
					it != data.end();
					it++) {
					cout << *it << "\t";
				}
				cout << endl;
			}
		}
	}
};

template <typename _result, class _parameter, class _head, class _tail>
class MultipleTests <_result, _parameter, utilities::Typelist<_head, _tail> >  {
public:
	static void test (_parameter & p, std::list<_result> & r) {
        _head t;
		r.insert(r.end(), t(p));
		MultipleTests<_result, _parameter, _tail>::test(p, r);
    }
};

template <typename _result, class _parameter, class _head>
class MultipleTests <_result, _parameter, utilities::Typelist<_head, utilities::NullType> >  {
public:
    static void test (_parameter & p, std::list<_result> & r) {
        _head t;
		r.insert(r.end(), t(p));
    }
    
};

};

#endif

