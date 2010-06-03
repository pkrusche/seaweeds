/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __SEAWEEDS_H__
#define __SEAWEEDS_H__

#include <algorithm>
#include <string>
#include <util/TypeList.h>

#include "bspcpp/tools/utilities.h"
#include "xasmlib/IntegerVector.h"

namespace seaweeds {

/**
 * \brief Seaweed algorithm implemented using comparison networks.
 * We compute full (four-way) implicit semi-local scores.
 * 
 * The returned seaweeds form distances which can be re-used as inputs
 * for incremental computation.
 * 
 * The starting points for the returned seaweeds are computed as follows.
 * 
 *   seaweeds_top[j] = k gives a seaweed from 
 *    j - k + 1/2 (on the top) -> j + 1/2 (on the bottom)
 * 
 *   seaweeds_right[j] = k gives a seaweed from 
 *    |y| - k - 1/2 (on the top) -> j + 1/2 (on the right) = |x| - j - 1/2 (on the bottom)
 */
template <size_t _omega = 8, size_t _bpc = 8,
		  class _permutation_container = utilities::IntegerVector<_omega> 
   >
class Seaweeds {
public:
	typedef utilities::IntegerVector<_bpc> string;
	typedef _permutation_container permutation_container;
	typedef utilities::IntegerVector<_omega> STATE_TYPE;

	/**
	 *  * The starting points for the returned seaweeds are computed as follows.
	 * 
	 *   seaweeds_top[j] = k gives a seaweed from 
	 *    j - k + 1/2 (on the top) -> j + 1/2 (on the bottom)
	 * 
	 *   seaweeds_right[j] = k gives a seaweed from 
	 *    |y| - k - 1/2 (on the top) -> j + 1/2 (on the right) = |x| + |y| - j - 1/2 (on the bottom)
	 * 
	 */
	 void operator()(string const & x, string const & y, 
					 permutation_container & seaweeds_right,
					 permutation_container & seaweeds_top,
					 bool use_right_input = false,
					 bool use_top_input = false
	 ) {		
		using namespace std;
		using namespace utilities;
		
		static const UINT64 msb  = STATE_TYPE::msb;
		static const UINT64 lsbs = STATE_TYPE::lsbs;

#ifdef _DEBUG_SEAWEEDS
		cout << "x = " << x << endl;
		cout << "y = " << y << endl;
#endif // _DEBUG_SEAWEEDS

		size_t  x_len = x.size(),
				y_len = y.size();

		size_t wf_len = min(x_len,y_len);
		size_t max_len = max(x_len,y_len);
		
		// state vectors are as long as the shorter string. we'll move around the 
		// part in which we actually need to do computation further below.
		STATE_TYPE left_storage (wf_len);
		STATE_TYPE top_storage (wf_len);
		// this is used as a buffer to store the characters from x and y we currently compare to
		STATE_TYPE x_text_storage (wf_len);
		STATE_TYPE y_text_storage (wf_len);

		// this vector stores the match mask
		STATE_TYPE matches_storage (wf_len);

		// temporary vectors
		STATE_TYPE tmp1_storage (wf_len);
		STATE_TYPE tmp2_storage (wf_len);

		// slices of these vectors to account for wavefront growth
		STATE_TYPE x_text(x_text_storage, 1);
		STATE_TYPE y_text(y_text_storage, 1);
		STATE_TYPE left(left_storage, 1);
		STATE_TYPE top(top_storage, 1);
		STATE_TYPE matches(matches_storage, 1);
		STATE_TYPE tmp1(tmp1_storage, 1);
		STATE_TYPE tmp2(tmp2_storage, 1);

		// if the given container is of the correct size, we take the 
		// values in it as inputs
		if (seaweeds_right.size() < x_len || !use_right_input) {
			// if the container is empty, resize and fill with defaults
			if(seaweeds_right.size() < x_len) {
				seaweeds_right.resize(x_len);
			}

			// we work with distances. in the default case, everything to the left
			// consists of matches.
			for (size_t j = 0; j < x_len; ++j) {
				seaweeds_right[j] = j+1;
			}
		} 
		
		if(seaweeds_top.size() < y_len || !use_top_input) {
			if (seaweeds_top.size() < y_len) {
				seaweeds_top.resize(y_len);
			}
			
			// if the container is empty, resize and fill with defaults
			for (size_t j = 0; j < y_len; ++j) {
				seaweeds_top[j] = 0;
			}
		}

		matches.one();
		top.one();
		left.one();

		left[0] = seaweeds_right[0];
		top[0] = seaweeds_top[0];
		x_text[0] = x[0];
		y_text[0] = y[0];

		int cur_wf_len = 1;

		for(size_t l = 1; l <= x_len + y_len - 1; ++l) {
			int pos = l - x_len;

			/* first, evaluate the transpositions we have atm. */
#ifdef _DEBUG_SEAWEEDS
			cout << "pos " << pos << " len = " << cur_wf_len << endl;
			cout << "pos " << pos << " x = " << x_text << endl;
			cout << "pos " << pos << " y = " << y_text << endl;
			cout << "pos " << pos << " l = " << left << endl;
			cout << "pos " << pos << " t = " << top << endl;
#endif
			// generate match mask
			matches.generate_match_mask(x_text, y_text);
			
			// backup
			tmp1 = top;
			tmp2 = left;

			// generate sorted version (all matches)
			top.cmpxchg(left);

			// whenever we have a match, the left seaweed gets translated
			// to the top
			top.replace_if(matches, tmp2);
			// ... and the top seaweed to the left
			left.replace_if(matches, tmp1);

			int carry = top.get(cur_wf_len - 1);
			int carry_r = left.get(0);

			left.saturated_inc();

			// grow wavefront.
			if(l < wf_len) {
				++cur_wf_len;

				x_text.resize(cur_wf_len);
				y_text.resize(cur_wf_len);
				left.resize(cur_wf_len);
				top.resize(cur_wf_len);
				matches.resize(cur_wf_len);
				tmp1.resize(cur_wf_len);
				tmp2.resize(cur_wf_len);
			}

			if(l < y_len){
				y_text <<= _omega;
				top <<= _omega;
				y_text[0] = y[l];
				top[0] = seaweeds_top[l];
			} else {
				x_text >>= _omega;
				left >>= _omega;
			}

			// we shift upwards while the wavefront is growing or staying at constant size
			// if x_len < y_len, this will cause the wavefront to move right while we shift
			// text and left
			if(l < x_len) {
				// still carryover from the left.
				left[cur_wf_len - 1] = seaweeds_right[l];

				// see if we still have new characters in x
				// if x is longer than wf_len, the wavefront will move
				// downwards for a number of steps.
				x_text[cur_wf_len - 1] = x[l];
			}

			if(l >= x_len && l >= y_len) {
				--cur_wf_len;
				x_text.resize(cur_wf_len);
				y_text.resize(cur_wf_len);
				left.resize(cur_wf_len);
				top.resize(cur_wf_len);
				matches.resize(cur_wf_len);
				tmp1.resize(cur_wf_len);
				tmp2.resize(cur_wf_len);
			}

			// if the wavefront has touched the bottom (i.e. after x_len-1 steps),
			// we start recording seaweed distances. Distance of -1 is invalid
			// (i.e. we have seaweed that went further than we can track), and 
			// makes sure all bits are set.
			if(l >= x_len) {
				// carry seaweed: the one which made it to the top.
				seaweeds_top[l-x_len] = (carry >= STATE_TYPE::lsbs) ? -1 : carry;
#ifdef _DEBUG_SEAWEEDS
				cout << "pos " << pos << " carry t " << pos - carry << "(dist " << seaweeds_top[l-x_len] << ")" << endl;
#endif // _DEBUG_SEAWEEDS
			}

			// once we have reached the right side of the alignment dag, start recording
			// output seaweeds on the right.
			if(l >= y_len) {
				seaweeds_right[l-y_len] = (carry_r >= STATE_TYPE::lsbs) ? -1 : carry_r;
#ifdef _DEBUG_SEAWEEDS
				cout << "pos " << pos << " carry r " << (int)y_len - carry_r - 1<< "(dist " << seaweeds_right[l-y_len] << ")" << endl;
#endif // _DEBUG_SEAWEEDS
			}
		}
#ifndef _NO_MMX
		utilities::do_emms();
#endif
    }
};

};

#endif
