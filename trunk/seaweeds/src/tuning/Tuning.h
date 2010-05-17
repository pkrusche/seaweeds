/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef TUNING_H_
#define TUNING_H_

#include <math.h>

#ifndef TN_SIZE
#define TN_SIZE 10000
#endif

#ifndef TN_MIN_N
#define TN_MIN_N 10
#endif

#ifndef TN_MAX_N
#define TN_MAX_N 1024*1024
#endif

#ifndef TN_CONFIRM
#define TN_CONFIRM 5
#endif

#ifndef TN_TOLERANCE
#define TN_TOLERANCE 0.5
#endif

template <class _Benchmark1,
		  class _Benchmark2,
		  class _parameter,
		  int _max = TN_MAX_N,
		  int _min = TN_MIN_N,
		  int _confirm = TN_CONFIRM
		 >
	class Tuning {
	public:
		static int tune(double _tolerance, _parameter & p, _parameter * p2 = NULL) {
			using namespace std;
			_parameter & _p2(p2 == NULL ? p : *p2);
			
			int num = _min;
			double t1, t2;
			int confirmcount = _confirm;

			int max = _max;
			int min = _min;
			bool findmax = true;

			_Benchmark1 benchmark1;
			_Benchmark2 benchmark2;
			for(;;) {
				t1 = benchmark1(num, p);
				t2 = benchmark2(num, _p2);

#ifdef _VERBOSE
				std::cout << num
						  << "\t" << min
						  << "\t" << max
						  << "\t" << t1
						  << "\t" << t2 << std::endl;
#endif
				if(findmax) {
					if(t2 < t1) {
						if(--confirmcount <= 0 || fabs(t1-t2) > _tolerance) {
							confirmcount = _confirm;
							max = num;
							findmax = false;
							num = min + (num - min) / 2;
						}
					} else {
						num = num * 3 / 2;
					}

					if(num > _max) {
						cerr << "No cutoff found, n too large." << endl;
						break;
					}
				} else {
					if(t2 < t1) {
						if(--confirmcount <= 0 || fabs(t1-t2) > _tolerance) {
							confirmcount = _confirm;
							max = num;
							num = min + (num - min) / 2;
						}
					} else if(t2 > t1) {
						if(--confirmcount <= 0 || fabs(t1-t2) > _tolerance) {
							confirmcount = _confirm;
							min = num;
							num = max - (max - num) / 2;
						}
					}

					if(max == min) {
						break;
					}
				}
			}
			return num;
		}
	};

#endif /*TUNING_H_*/
