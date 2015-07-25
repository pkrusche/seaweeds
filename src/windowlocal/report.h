/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __WL_REPORT_H__
#define __WL_REPORT_H__


#include <bsp_cpp/bsp_cpp.h> 

namespace windowlocal {

/** Window class: stores start positions and score of window */
class window : public bsp::ByteSerializable {
public:
	window(
		int _x0 = 0, 
		int _x1 = 0, 
		double _score = 0) : 
	x0(_x0), x1(_x1), score(_score) {}

	window(window const & rhs) {
		*this = rhs;
	}

	window const & operator=(window const & rhs) {
		if(&rhs != this) {
			x0 = rhs.x0;
			x1 = rhs.x1;
			score = rhs.score;
		}
		return *this;
	}

	/** implement bsp::ByteSerializable */
	void serialize(void * target, size_t nbytes) {
		char * tc = (char*) target;
		*((int*)tc) = x0;
		tc+= sizeof(int);
		*((int*)tc) = x1;
		tc+= sizeof(int);
		*((double*)tc) = score;
	}

	void deserialize(void * target, size_t nbytes) {
		char * tc = (char*) target;
		x0 = *((int*)tc);
		tc+= sizeof(int);
		x1 = *((int*)tc);
		tc+= sizeof(int);
		score = *((double*)tc);
	}

	size_t serialized_size() {
		return 2*sizeof(int) + sizeof (double);
	}

	int x0, x1;
	double score;
} 
#ifdef __GNUC__
__attribute__ ((aligned(8)))
#endif
;

/** interface for handing over window pairs to output/buffers */
struct window_reporter {
	virtual void report_score (window const &) = 0;
};

/** interface for handing over window pairs to output/buffers */
struct window_translator {
	/** overload to transform coordinates, and reject windows */
	virtual bool translate(window & w) {
		return true;
	}
};


};

#endif
