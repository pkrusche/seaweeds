/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "AlignmentPlot_Method.h"

// Define this to enable verification for each seaweed score that was obtained
// #define _SEAWEEDS_VERIFY

// This will enable more detailed debug output which can be used to reconstruct
// individual seaweed graphs
// #define _VERBOSETEST_WINDOWLCS

// This will output all window scores to stdout
// #define _VERBOSETEST_WINDOWOUTPUT

#include "datamodel/SequenceTranslation.h"
#include "windowlocal/seaweeds.h"

#include <boost/algorithm/string.hpp>


#include <tbb/mutex.h>

#ifndef SEAWEED_BPC
#define SEAWEED_BPC   8
#endif

// adjusted according to grid size and bpc
// 
// we need to trace seaweeds over a distance of 2*window_length + 1
// before they become indistinguishable.
// 
// Note that we use a grid size of two, and a 2x blow-up to do gap
// penalties of -1/2 which compensate for each other here.
#define MAX_W 		(((static_cast<UINT64>(1)) << (SEAWEED_BPC - 1)) - 1)

typedef windowlocal::SeaweedWindowLocalLCS<SEAWEED_BPC, SEAWEED_BPC> Seaweeds;

extern tbb::mutex ap_output_mutex;

class SeaweedNW;
struct _Ptr_Helper
{
	void operator() (SeaweedNW * ) {}
};

/** translate the scores for seaweed NW emulation */
class SeaweedNW : public AlignmentPlot_Method {
	public:
		SeaweedNW(AlignmentPlot & ap) : 
			AlignmentPlot_Method (ap),
			offset_x0(0), offset_x1(0), w(0)  {}

		/** implement windowlocal::window_translator */
		bool translate(windowlocal::window & w) {
			if( (w.x0&1) || (w.x1&1)) {
				return false;
			}
			w.x0 >>= 1;
			w.x1 >>= 1;

			// S(x,y) = LLCS(x',y') - 0.5*(|x| + |y|)
			// here, both x and y are of length w
			// so we get score = score - w
			w.score -= this->w;

			int tmp = w.x0;
			w.x0 = w.x1;
			w.x1 = tmp;

			w.x0+= offset_x0;
			w.x1+= offset_x1;

#ifdef _VERBOSETEST_WINDOWOUTPUT
			{
				tbb::mutex::scoped_lock l (ap_output_mutex);

				std::cout << w.x0 << "\t" << w.x1 << "\t" << w.score << std::endl;
			}
#endif
			return true;
		}

		/** implement AlignmentPlot_Method */
		void run(
			std::string const & s1, 
			std::string const & s2, 
			int offset1 = 0,
			int offset2 = 0) {

			using namespace std;
			using namespace bsp;
			using namespace boost;

			w = ap.get_windowlength();

			if(s1.length() < w) {
				bsp_abort("Input sequence is too short: %i < %i", s1.length(), w);
			}

			if (w > MAX_W) {
				bsp_abort("Maximum window length exceeded: %i > %i", w, MAX_W);				
			}

			offset_x0 = offset1;
			offset_x1 = offset2;

			std::string cmatch = "$";
			string s1_chars = "ACGTN_";
			string s2_chars = "ACGT_N";

			global_options.get("SeaweedNW::s1_chars", s1_chars, s1_chars);
			global_options.get("SeaweedNW::s2_chars", s2_chars, s2_chars);
			global_options.get("SeaweedNW::match", cmatch, cmatch);

			if (cmatch.length() < 1) {
				cmatch = "$";
			}

			s1_chars= cmatch + s1_chars;
			s2_chars= cmatch + s2_chars;

			Seaweeds::string s1_p = 
				datamodel::make_sequence<SEAWEED_BPC>(
					translate_input_sequence (
						to_upper_copy(s1.substr(0, w)), 
						cmatch[0]
					).c_str(), 
					s1_chars
				);
			Seaweeds::string s2_p = 
				datamodel::make_sequence<SEAWEED_BPC>(
					translate_input_sequence(
						to_upper_copy (s2),
						cmatch[0]
					).c_str(), 
					s2_chars );

			Seaweeds sw(w*2, s1_p, 2);
			ap.set_translator(boost::shared_ptr<windowlocal::window_translator>(
				this, _Ptr_Helper()));

			int pct_max = (int)s1.length() - w + 1;
			int lpc = 0;

			for (int i = 0; i < pct_max; ++i) {
				s1_p = 
					datamodel::make_sequence<SEAWEED_BPC>(
						translate_input_sequence (
							to_upper_copy(s1.substr(i, w)), 
							cmatch[0]
						).c_str(), 
						s1_chars
					);
				sw.set_pattern(s1_p);
				sw.count(s2_p, &ap, 0, 2*i);

				int tpc = i*100/pct_max;
				if(tpc > lpc) {
					lpc = tpc;
					std::cerr << ".";
				}
			}
			std::cerr << std::endl;
		}

	protected:
		/** translate the input sequence by introducing match characters */
		inline std::string translate_input_sequence(std::string const& in, char match = '$') {
			std::string out;
			int l = (int)in.length();
			out.resize(2*l);
			for (int i = 0; i < l; ++i) {
				out[2*i] = match;
				out[2*i+1] = in[i];
			}
			return out;
		}
	private:

		/** alignment plot offsets */
		int offset_x0;
		int offset_x1;

		/** window length from alignment plot */
		int w;
};

namespace {
	static struct _init {
		_init() {
			utilities::init_xasmlib();
			AlignmentPlot_Method::add_method<
				AlignmentPlot_Method_Generic_Factory<SeaweedNW > 
			> ("seaweednw");
		}
	} init;	
};
