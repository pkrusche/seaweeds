/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

/**
 * @file Histogram.h
 * @author Peter Krusche
 * 
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <boost/shared_array.hpp>

#include "bsp_cpp/Shared/SharedVariable.h"
#include "datamodel/TextIO.h"


namespace utilities {

	/** 
	 * Generic histogram class
	 */

	class Histogram : public bsp::Reduceable {
	public:

		enum {
			underflow = -1,
			overflow = -2,
		};


		Histogram(double _min = 0, double _max = 1, int _buckets = 100) : 
			min_val (_min), max_val(_max), nbuckets(_buckets), buckets(new int[_buckets+2])
		{
			ASSERT(_buckets > 1);
			ASSERT(_min < _max);
			memset(buckets.get(), 0, sizeof(int)*(nbuckets+2));
		}


		Histogram(Histogram const & h2) {
			min_val = h2.min_val;
			max_val = h2.max_val;
			nbuckets = h2.nbuckets;
			buckets = boost::shared_array<int>(new int[nbuckets + 2]);
			memcpy(buckets.get(), h2.buckets.get(), (2+nbuckets)*sizeof(int));
		}

		Histogram const & operator= (Histogram const & h2) {
			min_val = h2.min_val;
			max_val = h2.max_val;
			nbuckets = h2.nbuckets;
			buckets = boost::shared_array<int>(new int[nbuckets + 2]);
			memcpy(buckets.get(), h2.buckets.get(), (2+nbuckets)*sizeof(int));
			return *this;
		}

		void reset(double _min_val = 0, double _max_val = 1, int _nbuckets = 100) {
			min_val = _min_val;
			max_val = _max_val;

			if (nbuckets != _nbuckets) {
				nbuckets = _nbuckets;
				buckets = boost::shared_array<int>(new int[nbuckets + 2]);				
			}

			memset(buckets.get(), 0, sizeof(int)*(nbuckets+2));
		}

		/**
		 Hash a double value into buckets
		 */
		int hash(double val) const {
			double hv = (val-min_val)*(nbuckets)/(max_val-min_val) ;
			int ihv = (int)hv;

			if ( hv < 0 ) {
				ihv = underflow;
			} else if ( hv >= nbuckets ) {
				ihv = overflow;
			}
			return ihv + 2;
		}

		/**
		 Get approximate double value from buckets
		 */
		double unhash (int h) const {
			h-= 2;
			if (h >= nbuckets) {
				h = overflow;
			}
			if (h < 0) {
				if (h == underflow) {
					return -HUGE_VAL;
				} else {
					return  HUGE_VAL;
				}
			} else {
				double rv = h;

				rv = rv/nbuckets*(max_val-min_val);
				// center on bucket mean
				rv+= min_val + 1.0/(2.0*nbuckets);
				return rv;
			}
		}

		/** 
		Return the number of samples close to val
		*/
		int count(double val) {
			return buckets[hash(val)];
		}

		/**
		Add a number of double values
		 */
		Histogram const& add(double val, int count = 1) {
			buckets[hash(val)]+= count;
			return *this;
		}

		/**
		Combine with another histogram
		 */
		Histogram const& add(Histogram const & h) {
			using namespace std;
			Histogram hr(
				min(min_val, h.min_val), max(max_val, h.max_val), max(nbuckets, h.nbuckets) );

			for (int b = 0; b < nbuckets+2; ++b) {
				hr.add(unhash(b), buckets[b]);
			}

			for (int b = 0; b < h.nbuckets+2; ++b) {
				hr.add(h.unhash(b), h.buckets[b]);
			}
			*this = hr;
			return *this;
		}
		Histogram const& operator+=(Histogram const & h) {
			return add(h);
		}

		/** 
		Dump to stream
		*/
		void write(std::ostream & o, 
			bool header = true, char sep = '\n') {
			using namespace std;
			if (header) {
				o << "# Histogram data: " << sep;
				o << "# min=" << min_val << sep;
				o << "# max=" << max_val << sep;
				o << "# nbuckets=" << nbuckets << sep;
				o << "# n_underflow=" << buckets.get()[1] << sep;
				o << "# n_overflow=" << buckets.get()[0] << sep;
			}
			for (int i = 0; i < nbuckets; ++i) {
				o << buckets.get()[i+2] << sep;
			}
		}

		/** 
		read from a stream
		*/
		void read(std::istream & i, char sep = '\n') {
			using namespace std;
			using namespace TextIO;
			string line;
			int current_bucket = 0;
			while (getline(i, line, sep)) {
				line = trim (line);
				if (line[0] == '#') {
					line = trim(line, "\t #");
					std::vector<string> v;
					split(line, v, "=", "\t ");

					if (v.size() == 2) {
						if(v[0] == "min") {
							min_val = atof(v[1].c_str());
						} 
						if(v[0] == "max") {
							max_val = atof(v[1].c_str());
						}
						if(v[0] == "nbuckets") {
							nbuckets = atoi(v[1].c_str());
							buckets = boost::shared_array<int>(new int[nbuckets+2]);
							memset(buckets.get(), 0, sizeof(int)*(nbuckets+2));
						}
						if(v[0] == "n_underflow") {
							buckets[1] = atoi(v[1].c_str());
						}
						if(v[0] == "n_overflow") {
							buckets[0] = atoi(v[1].c_str());
						}
					}
				} else {
					if(current_bucket >= nbuckets) {
						return;
					}
					buckets[current_bucket+2] = atoi(line.c_str());
					++current_bucket;
				}
			}
		}

		/************************************************************************/
		/* Make histogram data serializable + reduceable                        */
		/************************************************************************/

		inline size_t serialized_size() {
			return (3+nbuckets)*sizeof(int) + 2*sizeof(double);
		}

		inline void serialize (void * target, size_t nbytes) {
			ASSERT (nbytes >= serialized_size());
			int * itarget = (int*)target;
			*itarget++ = nbuckets;
			double * dtarget = (double*) itarget;
			*dtarget++ = min_val;
			*dtarget++ = max_val;
			memcpy(dtarget, buckets.get(), sizeof(int)*(2+nbuckets));
		}

		inline void deserialize(void * source, size_t nbytes) {
			ASSERT (nbytes >= 3*sizeof(int) + 2*sizeof(double) );
			int * isource = (int*) source;
			int oldnb = nbuckets;
			nbuckets = *isource++;
			double * dsource = (double*)isource;
			min_val = *dsource++;
			max_val = *dsource++;
			isource = (int*)dsource;

			ASSERT (nbytes >= serialized_size() );
			if(oldnb != nbuckets) {
				buckets = boost::shared_array<int>(new int[nbuckets+2]);
			}
			memcpy(buckets.get(), isource, sizeof(int)*(2+nbuckets));
		}

		inline void reduce_with (Reduceable const * _rhs) {
			Histogram const * rhs = static_cast<Histogram const * >(_rhs);
			add(*rhs);
		}

		inline void make_neutral() {
			memset(buckets.get(), 0, sizeof(int)*(nbuckets+2));
		}

		/// Minimum value
		double min_val;

		/// Maximum value
		double max_val;
		
		/// number of buckets
		int nbuckets;
		
		/// bucket count
		boost::shared_array<int> buckets;
	};
};

#endif /* __HISTOGRAM_H__ */
