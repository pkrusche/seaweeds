/***************************************************************************
 *   Copyright (C) 2009 by Peter Krusche                                   *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

/**
 * @file AlignmentPlot.h
 * @author Peter Krusche
 * 
 */

#ifndef __ALIGNMENTPLOT_H__
#define __ALIGNMENTPLOT_H__

#include <boost/shared_ptr.hpp>

#include <bsp_cpp/bsp_cpp.h>

#include "../global_options.h"

#include "util/Histogram.h"
#include "util/FixedSizeQueue.h"

#include "windowlocal/report.h"

#ifndef ALIGNMENTPLOT_MAX_SIZE
#define ALIGNMENTPLOT_MAX_SIZE (1024*1024*1024)
#endif


/** 
 * Alignment plot class
 * 
 * Class to store window scores. Since full alignment plots can be very large, 
 * we use bucketing to limit the number of window pairs.
 * 
 * An alignment plot also keeps score histograms.
 */

class AlignmentPlot : 
	public windowlocal::window_reporter, 
	public bsp::Reduceable {
public:
	AlignmentPlot (
		int seq1_len = 1,
		int seq2_len = 1,
		int window_len = 0, 
		double min_score = 0,
		double max_score = 1, 
		int hbuckets = 1000
	) : 
	 m(seq1_len), 
	 n(seq2_len),
	 windowlength(window_len),
	 output_hist(min_score, max_score, hbuckets)
	{ 
		set_sizes();
		make_neutral();
	}

	AlignmentPlot (AlignmentPlot const & rhs) {
		*this = rhs;
	}

	AlignmentPlot const & operator=(AlignmentPlot const & rhs) {
		if (this != &rhs) {
			m = rhs.m;
			n = rhs.n;
			output_hist = rhs.output_hist;
			windows = rhs.windows;
			profile_a = rhs.profile_a;
			profile_b = rhs.profile_b;
			translate = rhs.translate;
			windowlength = rhs.windowlength;
		}
		return *this;
	}

	/** set sequence lengths*/
	inline void set_parameters(
		int _m, 
		int _n, 
		int _w, 
		double _min_score = 0,
		double _max_score = 1,
		int _b = 1000
		) {
		m = _m;
		n = _n;
		windowlength = _w;
		output_hist.reset(_min_score, _max_score, _b);
		set_sizes();
		make_neutral();
	}

	/** getters for the size parameters */
	int get_m() {
		return m;
	}

	int get_n() {
		return n;
	}

	int get_windowlength() {
		return windowlength;
	}

	/** get first profile */
	void get_profile_a(std::vector<double> & v) {
		v.resize(profile_a.capacity());
		for (int i = 0; i < profile_a.capacity(); ++i) {
			v[i] = profile_a[i];
		}
	}

	/** get second profile */
	void get_profile_b(std::vector<double> & v) {
		v.resize(profile_b.capacity());
		for (int i = 0; i < profile_b.capacity(); ++i) {
			v[i] = profile_b[i];
		}
	}

	/** get all sampled windows */
	void get_windows(std::vector<windowlocal::window> & v) {
		windows.get_all(v);
	}

	/** return reference to the score histogram */
	utilities::Histogram & get_score_histogram() {
		return output_hist;
	}

	/** set a translator */
	template <class _translate> 
	inline void set_translator() {
		translate = boost::shared_ptr<windowlocal::window_translator> (new _translate);
	}

	/** set a translator from another pointer */
	inline void set_translator(boost::shared_ptr<windowlocal::window_translator> const & t) {
		translate = t;
	}

	/** from windowlocal::window_reporter */
	void report_score (windowlocal::window const & win2) {
		windowlocal::window w (win2);

		if(translate.get() != NULL) {
			if (!translate->translate(w)) {
				return;
			}
		}

		output_hist.add(w.score);
		windows.enqueue(w.score, w);

		if (w.x0 >=0 && w.x0 < profile_a.capacity()) {
			using namespace std;
			profile_a[w.x0] = max(profile_a[w.x0], w.score);
		}

		if (w.x1 >=0 && w.x1 < profile_b.capacity()) {
			using namespace std;
			profile_b[w.x1] = max(profile_b[w.x1], w.score);
		}
	}
	
	/** from bsp::Reduceable */
	void make_neutral() {
		output_hist.make_neutral();
		profile_a.make_neutral();
		profile_b.make_neutral();
		windows.make_neutral();
	}

	/** from bsp::Reduceable */
	void reduce_with(Reduceable const * _rhs) {
		AlignmentPlot const * rhs = static_cast<AlignmentPlot const * > (_rhs);

		// cannot combine plots with different window lengths
		ASSERT(windowlength <= 0 || windowlength == rhs->windowlength);

		if(windowlength == 0) {
			// if the window length is 0, this plot hasn't been initialised.
			(*this) = (*rhs);
			return;
		}

		bool setsize = false;
		if (m < rhs->m) {
			m = rhs->m;
			setsize = true;
		}

		if (n < rhs->n) {
			n = rhs->n;
			setsize = true;
		}

		if (setsize) {
			set_sizes();
		}

		output_hist.reduce_with( &(rhs->output_hist) );
		windows.reduce_with(&(rhs->windows));
		profile_a.reduce_with(&(rhs->profile_a));
		profile_b.reduce_with(&(rhs->profile_b));
	}

	/** from bsp::ByteSerializable */
	size_t serialized_size() {
		return 	3*sizeof(int)
		 + profile_a.serialized_size() + sizeof(size_t)
		 + profile_b.serialized_size() + sizeof(size_t)
		 + output_hist.serialized_size() + sizeof(size_t)
		 + windows.serialized_size() + sizeof(size_t);
	}

	/** from bsp::ByteSerializable */
	void serialize(void * target, size_t nbytes) {
		int * tc = (int* ) target;
		*tc++ = m;
		*tc++ = n;
		*tc++ = windowlength;

		char * cc = (char*)tc;
		size_t sz = profile_a.serialized_size();
		*((size_t*)cc) = sz;
		cc+= sizeof(size_t);
		profile_a.serialize(cc, sz);
		cc+= sz;

		sz = profile_b.serialized_size();
		*((size_t*)cc) = sz;
		cc+= sizeof(size_t);
		profile_b.serialize(cc, sz);
		cc+= sz;

		sz = output_hist.serialized_size();
		*((size_t*)cc) = sz;
		cc+= sizeof(size_t);
		output_hist.serialize(cc, sz);
		cc+= sz;

		sz = windows.serialized_size();
		*((size_t*)cc) = sz;
		cc+= sizeof(size_t);
		windows.serialize(cc, sz);
	}

	/** from bsp::ByteSerializable */
	void deserialize(void * source, size_t nbytes) {
		int * tc = (int* ) source;
		m = *tc++;
		n = *tc++;
		windowlength = *tc++;

		char * cc = (char*)tc;
		size_t sz = *((size_t*)cc);
		cc+= sizeof(size_t);
		profile_a.deserialize(cc, sz);
		cc+= sz;

		sz = *((size_t*)cc);
		cc+= sizeof(size_t);
		profile_b.deserialize(cc, sz);
		cc+= sz;

		sz = *((size_t*)cc);
		cc+= sizeof(size_t);
		output_hist.deserialize(cc, sz);
		cc+= sz;

		sz = *((size_t*)cc);
		cc+= sizeof(size_t);
		windows.deserialize(cc, sz);
	}

private:

	/** fix sizes of profiles etc. */
	void set_sizes() {
		using namespace std;
		if (m < windowlength) {
			m = windowlength;
		}
		if (n < windowlength) {
			n = windowlength;
		}
		profile_a.resize(m - windowlength + 1);
		profile_b.resize(n - windowlength + 1);

		/** window factor and minimum number of windows are read from
		 *  global options
		 */
		static int window_factor 
			= bsp::global_option<int>("AlignmentPlot::window_factor", 20);
		static int min_windows 
			= bsp::global_option<int>("AlignmentPlot::min_windows", 1000);

		/** maximum number of windows: 1GB of data */
		static int max_windows = 
			bsp::global_option<int>("AlignmentPlot::max_window_storage", ALIGNMENTPLOT_MAX_SIZE) / 
			(sizeof(double) + sizeof(windowlocal::window));
		// std::cerr << "Setting max_windows = " << 
		// 	min( (m - windowlength + 1) * (n - windowlength + 1), // maximum number of windows we can get with m and n
		// 	min (max_windows, 
		// 	max( window_factor * max(m, n),  min_windows))) << std::endl;
		windows.set_max_size(
			min( (m - windowlength + 1) * (n - windowlength + 1), // maximum number of windows we can get with m and n
			min (max_windows, 
			max( window_factor * max(m, n),  min_windows))) );
	}

	/** sequence lengths for seq a and b */
	int m, n;

	/** alignment plot window length */
	int windowlength;

	/** window queue */
	utilities::FixedSizeQueue<windowlocal::window> windows;

	/** when not NULL, this is used to translate+filter coordinates and scores */
	boost::shared_ptr<windowlocal::window_translator> translate;

	/** profiles for sequence a and b */
	bsp::SharedArray<double, bsp::ReduceMax> profile_a;
	bsp::SharedArray<double, bsp::ReduceMax> profile_b;

	/** score histogram of all reported scores */
	utilities::Histogram output_hist;

	/** be friends with the I/O class */
	friend class AlignmentPlotIO;
};

#endif // __ALIGNMENTPLOT_H__
