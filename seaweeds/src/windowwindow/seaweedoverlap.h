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
#include "seaweeds/MultiSeaweeds.h"

#ifdef _HAVE_ATI_GPU
#include "gpulib/ATI_Gpulib.h"
#endif // _HAVE_ATI_GPU


namespace windowlcs {
	template<int _omega, int _bpc, class _report>
	class SeaweedOverlapMatcher : public utilities::Checkpointable {
	public:
		typedef typename seaweeds::ScoreMatrix<seaweeds::ImplicitStorage<seaweeds::Seaweeds<_omega, _bpc> > > scorematrix;
		typedef typename scorematrix::string string;
#ifdef _HAVE_ATI_GPU
		typedef seaweeds::MultiSeaweeds_GPU MultiSeaweeds_t;
#else
		typedef seaweeds::MultiSeaweeds<scorematrix> MultiSeaweeds_t;
#endif // _HAVE_ATI_GPU

		// each strip gets a score matrix
		typedef struct {
#ifdef _OVERLAP_VERIFY
			string x;
			string y;
#endif
			int m; int n;
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
		int multistrip_overlap(int strip_id) {		
			int p1_start = strip_id * real_overlap_size;

			// compute scorematrix for the shared bit
			scorematrix m(overlap_size, p_s2->size(), shared_strip_hsms.get_seaweedpermutation(strip_id));

			m.reverse_xy();

			std::vector<STRIP> v(n_strips);
			
			v[0].a = new typename scorematrix::archive(m.get_archive());
			v[0].m = m.get_m();
			v[0].n = m.get_n();
#ifdef _OVERLAP_VERIFY
			v[0].x = m.get_x();
			v[0].y = m.get_y();
#endif

			int cur_pos = p1_start + windowlength - overlap_size - step1;
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
			int extend_start = p1_start + windowlength;
			cur_pos = extend_start - overlap_size;
			for(int j = 0; j < n_strips; ++j) {
				scorematrix m(v[j].m, v[j].n, *v[j].a);
				delete v[j].a;
#ifdef _OVERLAP_VERIFY
				m.get_x() = v[j].x;
#endif
				m.reverse_xy();
				m.get_y() = string(*((string *)p_s2), p_s2->size());
				int extend_len = windowlength - m.get_m();
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
			int m   = p_s1->size();			
			int p = 0;

			// the shared portion of all strips
			if (overlap_size <= 0) {
				overlap_size = (int)min(
						(double)windowlength,
						::ceil(::sqrt(ceil((double)windowlength/step1))) * step1
						);
			}
			
			double t0 = utilities::time();
			double dtl = 0;

			strips_height = overlap_size;
			n_strips = (windowlength - strips_height) / step1 + 1;
			real_overlap_size = n_strips*step1;

			const int precomp_in_one_go = 400;
			int strip_id = 0;
			while (p <= m-windowlength) {
				if(strip_id%precomp_in_one_go == 0) {
					int overall_n_strips = min(precomp_in_one_go, (m - windowlength + step1 - 1) / step1);

					string * all_shared_strings  = new string[overall_n_strips];
					for (int j = 0; j < overall_n_strips; ++j) {
						all_shared_strings[j] = p_s1->substr(j*real_overlap_size + windowlength - overlap_size, overlap_size);
					}
					shared_strip_hsms.run(all_shared_strings, overall_n_strips, *p_s2);
					delete [] all_shared_strings;
				}

				p+= multistrip_overlap(strip_id % precomp_in_one_go);
				strip_id++;

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

		int strips_height;
		int n_strips;
		int real_overlap_size;

		int overlap_size;

		MultiSeaweeds_t shared_strip_hsms;
	};


	template <int _bpc, int _omega> 
	class SeaweedOverlapMatcherGenerator {
	public:
		typedef typename seaweeds::ScoreMatrix< seaweeds::ImplicitStorage<seaweeds::Seaweeds<_omega, _bpc> > > scorematrix;
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
