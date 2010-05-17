/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __WINDOWLOCAL_SEAWEEDS_H__
#define __WINDOWLOCAL_SEAWEEDS_H__

#include <string>
#include <loki/TypeManip.h>

#include "lcs/Llcs.h"
#include "xasmlib/IntegerVector.h"
#include "xasmlib/Queue.h"

namespace windowlocal {

template <size_t _bpc, class _reporter, size_t _omega>
class SeaweedWindowLocalLlcs {
public:
	typedef utilities::IntegerVector<_omega> STATE_TYPE;

	struct CopyInitializer {
		static void copyPattern(utilities::IntegerVector<_bpc> const & p, utilities::IntegerVector<_bpc> & target) {
			target = p;
		}
	};

	struct ResizeInitializer {
		static void copyPattern(utilities::IntegerVector<_bpc> const & p, utilities::IntegerVector<_omega> & target) {
			utilities::ResizeAlphabet<_bpc, _omega>
				(p, target);
		}
	};

	struct EndOfStripShiftDefault {
		void operator()(STATE_TYPE & s, INT64 offset) {
			// do nothing by default if omega does not divide 64 evenly
		}
	};

	struct EndOfStripShiftMM {
		void operator()(STATE_TYPE & s, INT64 offset) {
			s.shiftstart((int)offset);
		}
	};

	typedef typename Loki::Select<64%_omega == 0,
		EndOfStripShiftMM,
		EndOfStripShiftDefault>::Result EndOfStripShift_t;
	EndOfStripShift_t EndOfStripShift;

	typedef typename Loki::Select<_bpc == _omega,
		CopyInitializer,
		ResizeInitializer>::Result Initializer;

	SeaweedWindowLocalLlcs(size_t _window, utilities::IntegerVector<_bpc> const & _pattern)
		: window(_window) {
		ASSERT(log2(_window+1) <= _omega);

		Initializer::copyPattern(_pattern, pattern_storage);
#ifdef _SEAWEEDS_VERIFY
		pattern_orig = _pattern;
#endif // _SEAWEEDS_VERIFY
	}

	size_t operator()(
		utilities::IntegerVector<_bpc> const & text,
		_reporter * rpt = NULL, size_t stepsize = 1
	) {
		using namespace std;
		using namespace utilities;
		static const int msb  = ( static_cast<int>(1) << (_omega) );
		static const int lsbs = ( msb-1 );
		ASSERT(text.size() >= window);
		size_t p = pattern_storage.size(), t = text.size();

		// seaweeds will be tracked as long as they are still part
		// of the pattern wavefront, and while they are within the
		// window.
		int delta = lsbs - (int) ((window + p)/stepsize - 1);

		/* these vectors store the various bits of the state
		   of our seaweed automaton. we work on subintervals of
		   them to avoid computing seaweeds in the extended
		   alignment dag. */
		STATE_TYPE seaweeds_left_storage (p);
		STATE_TYPE seaweeds_top_storage (p);
		STATE_TYPE seaweeds_left_old_storage(p);
		STATE_TYPE seaweeds_top_old_storage(p);

		// storing the currently relevant bit of pattern
		STATE_TYPE pattern_temp_storage (p);

		// the current bit of text
		STATE_TYPE current_text_storage(p);

		// we will store the mask of matches here
		STATE_TYPE matches_storage(p);

		/* these are the parts of the above vectors we work on
		   they are grown and shrunk to only compute the parts
		   of the wavefront which are within the non-extended
		   alignment dag area */

		STATE_TYPE seaweeds_left     (seaweeds_left_storage, 1);
		STATE_TYPE seaweeds_top      (seaweeds_top_storage, 1);
		STATE_TYPE seaweeds_left_old (seaweeds_left_old_storage, 1);
		STATE_TYPE seaweeds_top_old  (seaweeds_top_old_storage, 1);
		STATE_TYPE current_text		 (current_text_storage, 1);
		STATE_TYPE matches			 (matches_storage, 1);
		STATE_TYPE pattern			 (pattern_temp_storage, 1);

		// the number of full subsequence matches
		size_t count = 0;

		// this priority queue contains the expiry positions of all
		// seaweeds which have reached the bottom
		// seaweeds 'expire' once they cannot affect the LCS
		// in the current window anymore (i.e. the window starting
		// position has moved past the starting position of the seaweed)
		utilities::Queue<int> bottom;

		// initial values
		seaweeds_left[0] = lsbs;
		pattern[0] = pattern_storage[0];

		for (size_t j = 0; j < t + p - 1; ++j) {
			// we need to subtract 2 because the first character is matched at j=0
			// and bottom window and wavefront share one cell
			int pos = (signed)j - (signed)window - (signed)p + 2;

			current_text <<= _omega;

			if(j < t) {
				// get next char of text
				current_text.put(0, text.get(j));
				// new seaweed enters from top
				seaweeds_top[0] = delta;
			}

			// create match/mismatch mapping by comparing
			matches.generate_match_mask(current_text, pattern);

			// save left and top seaweeds
			seaweeds_left_old = seaweeds_left;
			seaweeds_top_old = seaweeds_top;

			// seaweeds_top contains larger values
			// seaweeds_left contains smaller values
			seaweeds_top.cmpxchg(seaweeds_left);

			// for all matches, l_j becomes t_j
			seaweeds_left.replace_if(matches, seaweeds_top_old);

			// for all matches, t_j becomes l_(j-1)
			seaweeds_top.replace_if(matches, seaweeds_left_old);

			// save the seaweed that reaches the bottom
			int carry_sw;

			// check if we are on the part where the wavefront is growing
			// seaweeds which arrive before zero don't need to be recorded
			if(j < p-1) {
				carry_sw = lsbs;
			} else if(j < t) {
				// this is the branch for anywhere in the middle of the strip
				carry_sw = seaweeds_top.get(p-1);
			} else {
				// at the end of the strip, the wavefront becomes shorter again,
				// but we need to implement the step size of bits here
				carry_sw = seaweeds_top.get(seaweeds_top.size()-1);
			}

			// increase size of wavefront.
			if(j < p-1) {
				seaweeds_left.resize((size_t)j+2);
				seaweeds_top.resize((size_t)j+2);
				seaweeds_left_old.resize((size_t)j+2);
				seaweeds_top_old.resize((size_t)j+2);
				matches.resize((size_t)j+2);
				current_text.resize((size_t)j+2);
				pattern.resize((size_t)j+2);
				pattern[(size_t)j+1] = pattern_storage[(size_t)j+1];
				seaweeds_left[(size_t)j+1] = lsbs;
			}

			// -> move top seaweeds one down after replacing
			seaweeds_top <<= _omega;

			if(j >= t) {
				// we're at the other end where the wavefront becomes shorter
				int cells_remaining = (int)(p - j + t - 1);
				int excessive_cells = (int)seaweeds_top.size() - cells_remaining - 1;

				const int qwords_in_alignment = AVector<UINT64>::allocator::ALIGNMENT / 8;
				const int vwords_in_a_qword = qwords_in_alignment*64/_omega;

				excessive_cells/= vwords_in_a_qword;
				if(excessive_cells >= vwords_in_a_qword) {
					EndOfStripShift(seaweeds_left, qwords_in_alignment * excessive_cells);
					EndOfStripShift(seaweeds_top, qwords_in_alignment * excessive_cells);
					EndOfStripShift(seaweeds_left_old, qwords_in_alignment * excessive_cells);
					EndOfStripShift(seaweeds_top_old, qwords_in_alignment * excessive_cells);
					EndOfStripShift(matches, qwords_in_alignment * excessive_cells);
					EndOfStripShift(current_text, qwords_in_alignment * excessive_cells);
					EndOfStripShift(pattern, qwords_in_alignment * excessive_cells);
				}
			}

			// if seaweed has already started outside the window, we can ignore it
			int expiry_pos = (int)(pos + window + p - (signed)(carry_sw-delta)*(signed)stepsize);
			if(carry_sw < lsbs) {
				int rpos = (int) (t + p - expiry_pos);
				if(expiry_pos >= pos) {
					bottom.push(rpos);
				}
			}

#ifndef _NO_MMX
			utilities::do_emms();
#endif

			if(pos >= 0 && (pos & (stepsize - 1)) == 0) {
				// as we have moved the window forward, we can remove
				// all seaweeds which started before.
				bottom.pop((int)(t + p - pos - 2));

				size_t lcslen = 0;
				lcslen = window-bottom.size();
				if(lcslen == p)
					++count;
				if(rpt != NULL && (pos & (stepsize - 1)) == 0) {
					(*rpt)((size_t)pos, (double)lcslen);
				}

#ifdef _VERBOSETEST_WINDOWLCS
				if(carry_sw < lsbs) {
					cout << pos << "(" << t+p-pos << "/" << j << "), slcs = " << lcslen
						<< " carry seaweed d=" << carry_sw-delta << " exp at " << expiry_pos << "(" << t + p - expiry_pos << ")"
						<< " making critical point (" << expiry_pos << "," << pos + window << ")"
						<< "\tbottom = " << bottom
						<< endl;
				}
				cout << " " << pos << "(" << t+p-pos << ")" << " text   = " << current_text << endl;
				cout << " " << pos << "(" << t+p-pos << ")" << " mask   = " << matches << endl;
				cout << " " << pos << "(" << t+p-pos << ")" << " left   = " << seaweeds_left << endl;
				cout << " " << pos << "(" << t+p-pos << ")" << " top    = " << seaweeds_top << endl;
#endif
#ifdef _SEAWEEDS_VERIFY
				IntegerVector<_bpc> tmp_text;
				lcs::Llcs<IntegerVector<_bpc> > _lcs;
				size_t real_lcsl;
				tmp_text = text.substr(pos, window);
				real_lcsl = _lcs(pattern_orig, tmp_text);
				if(real_lcsl != lcslen) {
					cout << " " << pos << "(" << t+p-pos << ")" << " MISMATCH (real) " << real_lcsl << " vs (sw) " << lcslen << endl;
					cout << " " << pos << "(" << t+p-pos << ")" << " MISMATCH substr = " << tmp_text << endl;
					cout << " " << pos << "(" << t+p-pos << ")" << " MISMATCH pattern= " << pattern << endl;
					cout << " " << pos << "(" << t+p-pos << ")" << " MISMATCH bottom = " << bottom << endl;
				}
#endif // _SEAWEEDS_VERIFY
			}
			// increment distances
			// only need to do this for multiples of the
			// step size. seaweeds between steps don't need
			// to be distinguished
			if(std::abs((int)(pos& (stepsize - 1))) == stepsize - 1) {
				seaweeds_left.saturated_inc();
				seaweeds_top.saturated_inc();
			}

		}

		return count;
	}

private:
	size_t window;
	utilities::IntegerVector<_omega> pattern_storage;
#ifdef _SEAWEEDS_VERIFY
	utilities::IntegerVector<_bpc> pattern_orig;
#endif // _SEAWEEDS_VERIFY
};

};
#endif
