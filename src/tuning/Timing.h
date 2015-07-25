/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __TIMING_H__
#define __TIMING_H__

#include <math.h>
#include <vector>
#include <list>
#include <map>

#include "util/TypeList.h"
#include "bsp.h"

namespace tuning {

	typedef std::pair<const char *, std::vector<std::pair<double, double> > > TimingData;

typedef struct _TimingList {  
    int NUM;
    int add;
    int inc;
    
    std::list<TimingData> dataList;
} TimingList;
    
    
template <class _benchmark, class _parameter>
class Timing  {
public:
    
    const char * getName() {
        return typeid( _benchmark ).name();
    }
    
    /**
     * @brief Time for all num parameters from add, increasing by inc
     */
	void operator() (int add, int NUM, int inc, _parameter p, TimingData & data) {
		using namespace std;
		bsp_warmup( 2 );
		data.second.resize(NUM);
		// setup time
		data.first = getName();
		for(int num = 0; num < NUM; ++num) {
			// construction time
			double t0 = bsp_time();
			_benchmark benchmark(p);
			data.second[num].first = bsp_time() - t0;
			// computation time
			size_t actual = num*inc + add;
			data.second[num].second = benchmark(actual);
		}
	}
    
};

template <class _parameter, class _tlist> class MultiTiming;

template <typename _parameter, class _head, class _tail>
class MultiTiming <_parameter, utilities::Typelist<_head, _tail> >  {
public:
    static void getTimings (int add, int NUM, int inc, TimingList & timings, _parameter p) {
        _head t;
        TimingData tdata;
        t(add, NUM, inc, p, tdata);
        timings.dataList.insert(timings.dataList.end(), tdata);
        MultiTiming<_parameter, _tail>::getTimings(add, NUM, inc, timings, p);
    }
};

template <typename _parameter, class _head>
class MultiTiming <_parameter, utilities::Typelist<_head, utilities::NullType> >  {
public:
    static void getTimings (int add, int NUM, int inc, TimingList & timings, _parameter p) {
        _head t;
        TimingData tdata;
        t(add, NUM, inc, p, tdata);        
        timings.NUM = NUM;
        timings.inc = inc;
        timings.add = add;
        timings.dataList.insert(timings.dataList.end(), tdata);
    }
    
};

std::ostream & operator<< (std::ostream & o, TimingList & timings) {
    o << "n\t";
    
    for(std::list<TimingData>::iterator it = timings.dataList.begin(); it != timings.dataList.end(); ++it) {
            o << it->first << "\t\t";
    }
    o << std::endl;

    for(int num = 0; num < timings.NUM; ++num) {
            size_t actual = num*timings.inc + timings.add;
            
            o << actual << "\t";
            for(std::list<TimingData>::iterator it = timings.dataList.begin(); it != timings.dataList.end(); ++it) {
				o << it->second[num].first << "\t";
				o << it->second[num].second << "\t";
            }
            o << std::endl;
    }
    
    return o;
}

};

#endif
