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

// adjusted according to bpc
// 
#define MAX_W 		(((static_cast<UINT64>(1)) << (SEAWEED_BPC)) - 1)

typedef windowlocal::SeaweedWindowLocalLCS<SEAWEED_BPC, SEAWEED_BPC> Seaweeds;

extern tbb::mutex ap_output_mutex;

namespace {

	class SeaweedsAP;
	struct _Ptr_Helper
	{
		void operator() (SeaweedsAP * ) {}
	};

	/** translate the scores for seaweed NW emulation */
	class SeaweedsAP : public AlignmentPlot_Method {
		public:
			SeaweedsAP(AlignmentPlot & ap) : 
				AlignmentPlot_Method (ap),
				offset_x0(0), offset_x1(0)  {}

			/** implement windowlocal::window_translator */
			bool translate(windowlocal::window & w) {
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

				int w = ap.get_windowlength();

				if(s1.length() < w) {
					bsp_abort("Input sequence is too short: %i < %i", s1.length(), w);
				}

				if (w > MAX_W) {
					bsp_abort("Maximum window length exceeded: %i > %i", w, MAX_W);				
				}

				offset_x0 = offset1;
				offset_x1 = offset2;

				string s1_chars = "ACGTN_";
				string s2_chars = "ACGT_N";

				global_options.get("Seaweeds::s1_chars", s1_chars, s1_chars);
				global_options.get("Seaweeds::s2_chars", s2_chars, s2_chars);

				Seaweeds::string s1_p = 
					datamodel::make_sequence<SEAWEED_BPC>(
						to_upper_copy(s1.substr(0, w)).c_str(), 
						s1_chars
					);
				Seaweeds::string s2_p = 
					datamodel::make_sequence<SEAWEED_BPC>(
						to_upper_copy (s2).c_str(), 
						s2_chars );

				Seaweeds sw(w, s1_p, 1);
				ap.set_translator(boost::shared_ptr<windowlocal::window_translator>(
					this, _Ptr_Helper()));

				int pct_max = (int)s1.length() - w + 1;
				int lpc = 0;

				for (int i = 0; i < pct_max; ++i) {
					s1_p = 
						datamodel::make_sequence<SEAWEED_BPC>(
							to_upper_copy(s1.substr(i, w)).c_str(), 
							s1_chars
						);
					sw.set_pattern(s1_p);
					sw.count(s2_p, &ap, 0, i);

					int tpc = i*100/pct_max;
					if(tpc > lpc) {
						lpc = tpc;
						std::cerr << ".";
					}
				}
				std::cerr << std::endl;
			}

		private:

			/** alignment plot offsets */
			int offset_x0;
			int offset_x1;
	};

	static struct _init {
		_init() {
			utilities::init_xasmlib();
			AlignmentPlot_Method::add_method<
				AlignmentPlot_Method_Generic_Factory< SeaweedsAP > 
			> ("seaweeds");
		}
	} init;	
};
