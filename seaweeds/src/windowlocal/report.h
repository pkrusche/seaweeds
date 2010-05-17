/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __WL_REPORT_H__
#define __WL_REPORT_H__

namespace windowlocal {

template <typename score_t = double>
struct window {
	window(size_t _x0 = 0, size_t _x1 = 0, double _score = 0) : x0(_x0), x1(_x1), score(_score) {}

	size_t x0, x1;
	score_t score;
} 
#ifdef __GNUC__
__attribute__ ((aligned(8)))
#endif
;

template <typename score_t = double>
struct report_nothing {
	void operator() (window<score_t> const & win) {}
	void operator()(size_t pos, score_t score) {}
};


template <class _report_t, class integer = int>
struct reportscore {
	_report_t * report;
	size_t p0;

	reportscore(_report_t * _w, size_t _p0) : report(_w), p0(_p0) {}

	void operator()(size_t p1, integer score) {
		(*report)(window<integer>(p0, p1, score));
	}

	void flush() {

	}
};

template <class _report_t, class integer = int>
struct reportscore_with_buffer {
	_report_t * report;
	size_t p0;
	size_t buffer_pos;
	std::vector<window<integer> > buffer;

	reportscore_with_buffer(_report_t * _w, size_t _p0, size_t buffersize = 1024) : report(_w), p0(_p0) {
		window<integer> t;
		buffer.resize(buffersize);
		buffer_pos = 0;
	}

	~reportscore_with_buffer() {
		flush();
	}

	void operator()(size_t p1, integer score) {
		buffer[buffer_pos] = window<integer> (p0, p1, score);
		if(buffer_pos == buffer.size() - 1) {
			flush();
		} else {
			buffer_pos++;
		}
	}

	void flush() {
		for (size_t s = 0; s < buffer_pos; ++s) {
			(*report)(buffer[s]);
		}
		buffer_pos = 0;
		report->flush();
	}
};


};

#endif
