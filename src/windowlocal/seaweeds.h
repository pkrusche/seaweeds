/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __WINDOWLOCAL_SEAWEEDS_H__
#define __WINDOWLOCAL_SEAWEEDS_H__

#include "autoconfig.h"

#include <string>

#include "util/TypeList.h"
#include "lcs/Llcs.h"
#include "xasmlib/IntegerVector.h"
#include "xasmlib/Queue.h"

#include "report.h"

namespace windowlocal {

template <size_t _bpc, size_t _omega>
class SeaweedWindowLocalLCS {
public:
	typedef utilities::IntegerVector<_bpc> string;
	typedef utilities::IntegerVector<_omega> STATE_TYPE;

	enum {
		max_windowlength = string :: msb - 1,
	};

	struct CopyInitializer {
		static void copyPattern(string const & p, string & target) {
			target = p;
		}
	};

	struct ResizeInitializer {
		static void copyPattern(string const & p, utilities::IntegerVector<_omega> & target) {
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

	typedef typename utilities::Select<64%_omega == 0,
		EndOfStripShiftMM,
		EndOfStripShiftDefault>::Result EndOfStripShift_t;
	EndOfStripShift_t EndOfStripShift;

	typedef typename utilities::Select<_bpc == _omega,
		CopyInitializer,
		ResizeInitializer>::Result Initializer;

	SeaweedWindowLocalLCS(size_t _window, string const & _pattern, size_t _grid_size = 1)
		: window(_window), grid_size(_grid_size) {
		Initializer::copyPattern(_pattern, pattern_storage);
#ifdef _SEAWEEDS_VERIFY
		pattern_orig = _pattern;
#endif // _SEAWEEDS_VERIFY
	}

	int count(string const & text, 
		window_reporter * rpt = NULL, 
		int text_p0 = 0,
		int pat_p0 = 0
		) {
		using namespace std;
		using namespace utilities;
		static const int msb  = ( static_cast<int>(1) << (_omega) );
		static const int lsbs = ( msb-1 );
		size_t p = pattern_storage.size(), t = text.size();

		ASSERT(t >= window);
		ASSERT((p + window)/grid_size <= 2*max_windowlength);
		ASSERT(window % grid_size == 0);

		// necessary since we may not know how often a value was incremented on 
		// its way down.
		ASSERT(p % grid_size == 0);

		/* these vectors store the various bits of the state
		   of our seaweed automaton. we work on subintervals of
		   them to avoid computing seaweeds in the extended
		   alignment dag. */
		STATE_TYPE seaweeds_left_storage (p);
		STATE_TYPE seaweeds_top_storage (p);

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
		
		// done in loop
		// seaweeds_top[0] = 0;
		// current_text.put(0, text.get(0));
		
		pattern[0] = pattern_storage[0];

		int pos = - (signed)window - (signed)p + 2;
		int j = 0;

		// save the seaweed that reaches the bottom
		int carry_sw = lsbs;
		int sw_start = -1000, sw_end = -10000;
		int expiry_pos = 0;

#ifdef _VERBOSETEST_WINDOWLCS
		cout << "|t| = " << t << endl;
		cout << "t   = " << text << endl;
		cout << "|p| = " << p << endl;
		cout << "p   = " <<  pattern_storage << endl;
		cout << "w   = " << window << endl;
#endif

		while(pos <= (int)t - (int)window) {
			int current_on_top = (int)(pos+p + window-2);

			// pos+p => position of first cell on top
			bool inc_this_step = (current_on_top & (grid_size-1)) == grid_size-1;

			// window and bottom end of wavefront share 1 cell
			// start and top end of wavefront, too.
			if(j < t) {
				// get next char of text
				current_text.put(0, text.get(j));
				// new seaweed enters from top
				seaweeds_top[0] = 0;
			}

			// create match/mismatch mapping by comparing
			matches.generate_match_mask(current_text, pattern);

			// seaweeds_top contains larger values
			// seaweeds_left contains smaller values
			seaweeds_top.cmpxchg_masked(seaweeds_left, matches);

			// check if we are on the part where the wavefront is growing
			// seaweeds which arrive before zero don't need to be recorded
			if(j < p-1) {
				carry_sw = lsbs;
			} else {
				// at the end of the strip we might be resizing this
				carry_sw = seaweeds_top.get(seaweeds_top.size()-1);				
			}

			// current location of bottom cell 
			int current_on_bottom = (int)(pos+window-1);

			// at this point we must increment the distance once, 
			// since we expect this in our distance calculation
			// and it's only done below for the entire wavefront
			if (inc_this_step) {
				++carry_sw;
			}

			// if seaweed has already started outside the window, we can ignore it

			// calculate start position: seaweed has been incremented p times 
			// to get to the bottom, 
			int distance = (int)(carry_sw * grid_size - p);

			sw_start = current_on_bottom - distance;
			sw_start -= sw_start & (grid_size-1);
			sw_end   = (current_on_bottom - (current_on_bottom&(grid_size-1)));

			expiry_pos = (int)(sw_start + window);

			if( carry_sw < lsbs
			&&  expiry_pos > current_on_bottom) {
				bottom.push(-expiry_pos);
			}
			bottom.pop(-current_on_bottom);

#ifdef _VERBOSETEST_WINDOWLCS
			cout << "_____ " << pos << " / b:" << current_on_bottom << " t:" << current_on_top
				 << (inc_this_step ? " ++ " : " == ")
				 << " _____________________" << endl;
			cout << " " << pos  << " ________ text   = " << current_text << endl;
			cout << " " << pos  << " ________ pattern= " << pattern << endl;
			cout << " " << pos  << " ________ mask   = " << matches << endl;
			cout << " " << pos  << " ________ left   = " << seaweeds_left << endl;
			cout << " " << pos  << " ________ top    = " << seaweeds_top << endl;
			if(carry_sw < lsbs) {
				cout << " " << pos  << " ________ sw_start = " << sw_start << endl;
				cout << " " << pos  << " ________ carry_sw = " << carry_sw << "/" << distance << " exp at " << expiry_pos << " (lsbs:" << lsbs << ")" << endl;
				cout << " " << pos  << " ________ slcs = " << ((int)window-(int)bottom.size())
					<< " critical point (" << sw_start << "," << sw_end << ")"
					<< "\tbottom = " << bottom
					<< endl;
			} else {
				cout << " " << pos  << " ________ sw_start = -inf" << endl;
				cout << " " << pos  << " ________ carry_sw = +inf" << endl;
				cout << " " << pos  << " ________ slcs = " << ((int)window-(int)bottom.size())
					<< " critical point (-inf," << sw_end << ")"
					<< "\tbottom = " << bottom
					<< endl;
			}
			cout << " " << pos  << " ________ bottom = " << bottom << endl;
			ASSERT(sw_end >= sw_start);
#endif

			// move right one step 
			// increase size of wavefront.
			if(j < p-1) {
				seaweeds_left.resize((size_t)j+2);
				seaweeds_top.resize((size_t)j+2);
				matches.resize((size_t)j+2);
				current_text.resize((size_t)j+2);
				pattern.resize((size_t)j+2);
				pattern[(size_t)j+1] = pattern_storage[(size_t)j+1];
				seaweeds_left[(size_t)j+1] = lsbs;
			}

			// -> move top seaweeds one down after replacing
			seaweeds_top <<= _omega;
			current_text <<= _omega;

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
					EndOfStripShift(matches, qwords_in_alignment * excessive_cells);
					EndOfStripShift(current_text, qwords_in_alignment * excessive_cells);
					EndOfStripShift(pattern, qwords_in_alignment * excessive_cells);
				}
			}
			++pos;
			++j;

			if (inc_this_step) {
				seaweeds_top.saturated_inc();
				seaweeds_left.saturated_inc();
			}

#ifndef _NO_MMX
			utilities::do_emms();
#endif
			// pos is already incremented for the next step
			if(pos > 0 && (((pos-1) & (grid_size-1)) == 0)) {
				// as we have moved the window forward, we can remove
				// all seaweeds which started before.
				int lcslen = 0;
				lcslen = (int)window-(int)bottom.size();
				if(lcslen == p)
					++count;

#ifdef _SEAWEEDS_VERIFY
				IntegerVector<_bpc> tmp_text;
				lcs::Llcs<string> _lcs;
				size_t real_lcsl;
				tmp_text = text.substr(pos-1, window);
				real_lcsl = _lcs(pattern_orig, tmp_text);
				if(real_lcsl != lcslen) {
					cout << " " << pos-1  << " MISMATCH (real) " << real_lcsl << " vs (sw) " << lcslen << endl;
#ifdef _SEAWEEDS_VERIFY_THROW
					throw "Failed to verify result!";
#endif
				} else 
#endif // _SEAWEEDS_VERIFY
				if(rpt != NULL) {
					rpt->report_score(windowlocal::window ((int)pos-1+text_p0, pat_p0, (double)lcslen));
				}

			}

		}

		return (int)count;
	}

	/** set the pattern */
	void set_pattern(string _pattern) {
		Initializer::copyPattern(_pattern, pattern_storage);
#ifdef _SEAWEEDS_VERIFY
		pattern_orig = _pattern;
#endif // _SEAWEEDS_VERIFY
	}

	/* set the window length */ 
	void set_windowlength(int _windowlength) {
		window = _windowlength;
	}

private:
	/** window length */
	size_t window;

	/** grid accuracy -- only report seaweeds at %grid_size intervals */
	size_t grid_size;

	/** here we store the pattern expanded to _omega bits per char */
	utilities::IntegerVector<_omega> pattern_storage;
#ifdef _SEAWEEDS_VERIFY
	string pattern_orig;
#endif // _SEAWEEDS_VERIFY
};

};
#endif
