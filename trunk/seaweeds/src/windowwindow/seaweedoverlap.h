/***************************************************************************
 *   Copyright (C) 2009 by Peter Krusche                                   *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef seaweedoverlap_h__
#define seaweedoverlap_h__

#include <vector>
#include <cmath>
#include <algorithm>

#include "xasmlib/IntegerVector.h"
#include "checkpoint/checkpoint.h"
#include "seaweeds/ScoreMatrix.h"

namespace windowlcs {
	template<int _omega, int _bpc, class _report>
	class SeaweedOverlapMatcher : public utilities::Checkpointable {
	public:
		typedef typename seaweeds::ScoreMatrix<seaweeds::ImplicitStorage<seaweeds::Seaweeds<_omega, _bpc> > > scorematrix;
		typedef typename scorematrix::string string;

		// each strip gets a score matrix
		typedef struct {
#ifdef _OVERLAP_VERIFY
			string x;
			string y;
#endif
			size_t m; size_t n;
			typename scorematrix::archive * a;
		} STRIP;


		SeaweedOverlapMatcher(int _os = -1) : overlap_size (_os) {}

		virtual void checkpoint_loadstate(utilities::ParameterFile & p) {
		}

		virtual void checkpoint_savestate(utilities::ParameterFile & p) {
		}

		virtual void new_checkpointer(utilities::Checkpointer * p) {
		}

		/**
		 * \brief multistrip overlap computation
		 *  ideally, overlap_size and windowlength should be multiples of step1
		 * \return number characters in p_1 actually covered
		 */
		size_t multistrip_overlap(size_t p1_start) {
			size_t strips_height = overlap_size;
			size_t n_strips = (windowlength - strips_height) / step1 + 1;
			size_t real_overlap_size = n_strips*step1;
			
			// compute scorematrix for the shared bit
			scorematrix m(overlap_size, p_s2->size());
			string shared_substring = p_s1->substr(p1_start + windowlength - overlap_size, overlap_size);
			m.semilocallcs(shared_substring, *p_s2);
			m.reverse_xy();

			std::vector<STRIP> v(n_strips);
			
			v[0].a = new typename scorematrix::archive(m.get_archive());
			v[0].m = m.get_m();
			v[0].n = m.get_n();
#ifdef _OVERLAP_VERIFY
			v[0].x = m.get_x();
			v[0].y = m.get_y();
#endif

			size_t cur_pos = p1_start + windowlength - overlap_size - step1;
			// extend towards the top
			for(int j = 1; j < n_strips; ++j) {
				string add_substring = p_s1->substr(cur_pos, step1);
				add_substring.reverse();
				m.incremental_semilocallcs(add_substring, scorematrix::APPEND_TO_X);
				v[j].a = new typename scorematrix::archive(m.get_archive());
				v[j].m = m.get_m();
				v[j].n = m.get_n();
#ifdef _OVERLAP_VERIFY
				v[j].x = m.get_x();
				v[j].y = m.get_y();
#endif
				cur_pos-= step1;
			}

			// extend towards bottom and query
			size_t extend_start = p1_start + windowlength;
			cur_pos = extend_start - overlap_size;
			for(int j = 0; j < n_strips; ++j) {
				scorematrix m(v[j].m, v[j].n, *v[j].a);
				delete v[j].a;
#ifdef _OVERLAP_VERIFY
				m.get_x() = v[j].x;
#endif
				m.reverse_xy();
				m.get_y() = string(*((string *)p_s2), p_s2->size());
				size_t extend_len = windowlength - m.get_m();
				if(extend_len > 0) {
					string add_substring = p_s1->substr(extend_start, extend_len);
					m.incremental_semilocallcs(add_substring, scorematrix::APPEND_TO_X);
				}
				windowlocal::reportscore<_report, double> rpt(reporter, cur_pos);
				m.query_y_windows(windowlength, step2, &rpt);

#ifdef _OVERLAP_VERIFY
				string real_substring = p_s1->substr(cur_pos, windowlength);
				scorematrix m2(windowlength, p_s1->size());
				m2.semilocallcs(real_substring, *p_s2);
				if (!m2.equals(m)) {
					std::cerr << "ERROR verifying in row " << cur_pos << endl;
				} else {
					std::cout << "Substr  " << cur_pos << "(" << windowlength << ") is ok." << endl;
				}
#endif
				cur_pos-= step1;
			}
			return real_overlap_size;
		}

		void run() {
			using namespace std;
			size_t m   = p_s1->size();			
			size_t p = 0;

			// the shared portion of all strips
			if (overlap_size <= 0) {
				overlap_size = (int)min(
						(double)windowlength,
						::ceil(::sqrt(ceil((double)windowlength/step1))) * step1
						);
			}
			
			double t0 = utilities::time();
			double dtl = 0;

			while (p <= m-(signed)windowlength) {
				p+= multistrip_overlap(p);
				double dt = utilities::time() - t0;

				if(dt - dtl > 5) {
					std::cerr << "Completed [" << p/step1 << "/"<< (m-windowlength)/step1 << "], time remaining " 
						<< dt/(p+1)*(m-windowlength) - dt  << "s" << std::endl;
					dtl = dt;
				}

			}
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
		}

	private:
		string const * p_s1;
		string const * p_s2;

		size_t windowlength;
		size_t threshold;
		size_t step1;
		size_t step2;
		_report * reporter;

		int overlap_size;
	};


	template <int _bpc, int _omega> 
	class SeaweedOverlapMatcherGenerator {
	public:
		typedef typename seaweeds::ScoreMatrix<seaweeds::ImplicitStorage<seaweeds::Seaweeds<_omega, _bpc> > > scorematrix;
		typedef typename scorematrix::string string;

		SeaweedOverlapMatcherGenerator() {}

		template <class _reporter>
		utilities::Checkpointable * operator()(string const & s1, string const & s2, 
			size_t windowlength, 
			size_t threshold = 0, 
			size_t step1 = 1, 
			size_t step2 = 1,
			_reporter * r = NULL, 
			int overlap_size = -1
			) {
				SeaweedOverlapMatcher<_omega, _bpc, _reporter> * swm = new SeaweedOverlapMatcher<_omega, _bpc, _reporter>(overlap_size);
				swm->match (s1, s2, windowlength, threshold, step1, step2, r);
				return (utilities::Checkpointable*)swm;
		}
	};
};

#endif // seaweedoverlap_h__
