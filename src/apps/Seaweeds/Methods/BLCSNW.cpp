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
#include "windowlocal/naive_cipr.h"

#include <boost/algorithm/string.hpp>


#include <tbb/mutex.h>

#ifndef BLCS_BPC
#define BLCS_BPC   8
#endif

typedef windowlocal::BPWindowLocalLCS <BLCS_BPC> Matcher;

extern tbb::mutex ap_output_mutex;

namespace {
	class BLCSNW;
	struct _Ptr_Helper
	{
		void operator() (BLCSNW * ) {}
	};

	/** translate the scores for seaweed NW emulation */
	class BLCSNW : public AlignmentPlot_Method {
		public:
			BLCSNW(AlignmentPlot & ap) : 
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

				offset_x0 = offset1;
				offset_x1 = offset2;

				std::string cmatch = "$";
				string s1_chars = "ACGTN_";
				string s2_chars = "ACGT_N";

				global_options.get("BLCSNW::s1_chars", s1_chars, s1_chars);
				global_options.get("BLCSNW::s2_chars", s2_chars, s2_chars);
				global_options.get("BLCSNW::match", cmatch, cmatch);

				if (cmatch.length() < 1) {
					cmatch = "$";
				}

				s1_chars= cmatch + s1_chars;
				s2_chars= cmatch + s2_chars;

				Matcher::string s1_p = 
					datamodel::make_sequence<BLCS_BPC>(
						translate_input_sequence (
							to_upper_copy(s1.substr(0, w)), 
							cmatch[0]
						).c_str(), 
						s1_chars
					);
				Matcher::string s2_p = 
					datamodel::make_sequence<BLCS_BPC>(
						translate_input_sequence(
							to_upper_copy (s2),
							cmatch[0]
						).c_str(), 
						s2_chars );

				Matcher sw(w*2, s1_p);
				ap.set_translator(boost::shared_ptr<windowlocal::window_translator>(
					this, _Ptr_Helper()));

				int pct_max = (int)s1.length() - w + 1;
				int lpc = 0;

				for (int i = 0; i < pct_max; ++i) {
					s1_p = 
						datamodel::make_sequence<BLCS_BPC>(
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

	static struct _init {
		_init() {
			utilities::init_xasmlib();
			AlignmentPlot_Method::add_method<
				AlignmentPlot_Method_Generic_Factory<BLCSNW > 
			> ("blcsnw");
		}
	} init;	
};
