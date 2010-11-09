/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __WINDOWWINDOW_BOASSON_H__
#define __WINDOWWINDOW_BOASSON_H__

#include "checkpoint/checkpoint.h"

#include "seaweeds/ScoreMatrix.h"

#include "windowlocal/naive.h"
#include "windowlocal/naive_cipr.h"
#include "windowlocal/seaweeds.h"
#include "windowlocal/scorematrix.h"
#include "windowlocal/report.h"

namespace windowlcs {

template <class string, class stripmatcher, class _report > 
class StripWindowWindowMatcher : utilities::Checkpointable {
public:
	StripWindowWindowMatcher() : p_s1(NULL), p_s2(NULL), windowlength(100), step1(1), step2(1), threshold(0), reporter(NULL), s_p0(-1) {}

	virtual void checkpoint_savestate(utilities::ParameterFile & pf) {
		pf.set("p0", (int)s_p0);
	}

	virtual void checkpoint_loadstate(utilities::ParameterFile & pf) {
		s_p0 = 0;
		pf.get("p0", (int&)s_p0, 0);
	}

	virtual void run() {
		// check if computation is finished already, or if not initialized
		if(s_p0 < 0)
			return;

		INT64 m = p_s1->size();
		INT64 n = p_s2->size();
		double t0 = utilities::time();
		double dtl = 0;

		if(s_p0 > 0) {
			std::cout << "[resume] starting at position " << s_p0/step1 << std::endl;
		}

		windowlocal::reportscore<_report, double> _rpt(reporter, (int)s_p0);
		for(UINT64 p0 = s_p0; p0 <= m-windowlength; p0+= step1) {
			string sub1 = p_s1->substr((size_t) p0, windowlength);
			stripmatcher _sm (windowlength, sub1);
			_rpt.p0 = (int) p0;
			if(reporter != NULL) {
				_sm(*p_s2, &_rpt, step2);
			} else {
				_sm(*p_s2, NULL, step2);
			}

			s_p0 = p0;
			checkpoint();

			double dt = utilities::time() - t0;
			
			if(dt - dtl > 5) {
				std::cout << "Completed [" << p0/step1 << "/"<< (m-windowlength+1)/step1 << "], time remaining " 
					<< dt/(p0+1)*(m-windowlength+1) - dt  << "s" << std::endl;
				dtl = dt;
			}
		}
		_rpt.flush();
		// this sets the checkpoint to signal that we have finished the computation
		s_p0 = -1;
		checkpoint();
	}

	void match(string const & _s1, string const & _s2, 
					size_t _windowlength, 
					size_t _threshold = 0, 
					size_t _step1 = 1, 
					size_t _step2 = 1,
					_report * _rpt = NULL
					) {
		p_s1 = &_s1;
		p_s2 = &_s2;
		windowlength = _windowlength;
		threshold = _threshold;
		step1 = _step1;
		step2 = _step2;
		reporter = _rpt;
		s_p0 = 0;
	}

private:
	string const * p_s1;
	string const * p_s2;

	size_t windowlength;
	size_t threshold;
	size_t step1;
	size_t step2;
	_report * reporter;

	INT64 s_p0;
};

template<size_t _bpc, size_t _omega = 16> 
struct SeaweedWindowWindowMatcherGenerator {
	typedef utilities::IntegerVector<_bpc> string;

	template <class _report >
	utilities::Checkpointable * operator()(string const & s1, string const & s2, 
					size_t windowlength, 
					size_t threshold = 0, 
					size_t step1 = 1, 
					size_t step2 = 1,
					_report * r = NULL
					) {
		StripWindowWindowMatcher <
			string, 
			windowlocal::SeaweedWindowLocalLlcs<
				_bpc, 
				windowlocal::reportscore<
					_report, double
				>, 
				_omega
			>,
			_report
		> * swm = new StripWindowWindowMatcher <string, windowlocal::SeaweedWindowLocalLlcs<_bpc, windowlocal::reportscore<
						_report, double	>, 	_omega	>, _report> ();
		swm->match (s1, s2, windowlength, threshold, step1, step2, r);
		return (utilities::Checkpointable*)swm;
	}	
};		
		
template<size_t _bpc> 
struct NaiveCIPRWindowWindowMatcherGenerator {
	typedef utilities::IntegerVector<_bpc> string;
	template <class _report >
	utilities::Checkpointable * operator()(string const & s1, string const & s2, 
					size_t windowlength, 
					size_t threshold = 0, 
					size_t step1 = 1, 
					size_t step2 = 1,
					_report * r = NULL
					) {
		StripWindowWindowMatcher <
			string, 
			windowlocal::NaiveBitparallelWindowLocalLlcs<
				_bpc, 
				windowlocal::reportscore<
					_report, double
				> 
			>,
			_report
		> * swm = new StripWindowWindowMatcher <
			string, 
			windowlocal::NaiveBitparallelWindowLocalLlcs<
				_bpc, 
				windowlocal::reportscore<
					_report, double
				> 
			>,
			_report
		> ();
		swm->match (s1, s2, windowlength, threshold, step1, step2, r);
		return (utilities::Checkpointable*)swm;
	}	
};		

template<class _string, class llcs> 
struct NaiveWindowWindowMatcherGenerator {
	typedef _string string;
	template <class _report >
	utilities::Checkpointable * operator()(string const & s1, string const & s2, 
					size_t windowlength, 
					size_t threshold = 0, 
					size_t step1 = 1, 
					size_t step2 = 1,
					_report * r = NULL
					) {
		StripWindowWindowMatcher <
			string, 
			windowlocal::NaiveWindowLocalLlcs<
				llcs, 
				string,
				windowlocal::reportscore<
					_report, double
				> 
			>,
			_report
		> * swm = new StripWindowWindowMatcher <
			string, 
			windowlocal::NaiveWindowLocalLlcs<
				llcs, 
				string,
				windowlocal::reportscore<
					_report, double
				> 
			>,
			_report
		> ();
		swm->match (s1, s2, windowlength, threshold, step1, step2, r);
		return (utilities::Checkpointable*)swm;
	}	
};

template<size_t _bpc, size_t _omega>
struct ScorematrixWindowWindowMatcherGenerator {
	typedef seaweeds::ScoreMatrix< seaweeds::ImplicitStorage<seaweeds::Seaweeds<_omega, _bpc> > > scorematrix;
	typedef typename scorematrix::string string;

	template <class _report >
	utilities::Checkpointable * operator()(string const & s1, string const & s2,
					size_t windowlength,
					size_t threshold = 0,
					size_t step1 = 1,
					size_t step2 = 1,
					_report * r = NULL
					) {
		typedef StripWindowWindowMatcher<
		string,
		windowlocal::ScoreMatrixWindowLocalLlcs<
		windowlocal::reportscore<
							_report, double
						>,
		scorematrix
		>, _report > swm_t;
		swm_t * swm = new swm_t();
		swm->match (s1, s2, windowlength, threshold, step1, step2, r);
		return (utilities::Checkpointable*)swm;
	}
};

};

#endif
