/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef windowwindow_factory_h__
#define windowwindow_factory_h__

#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"
#include "lcs/RationalScores.h"

#include "windowwindow/windowwindowlcs.h"
#include "windowwindow/seaweedoverlap.h"
#include "windowwindow/translate_and_print.h"

#include "checkpoint/checkpoint.h"
#include "bspcpp/bsp_cpp.h"

#include <boost/program_options.hpp>


namespace windowlocal {
	template <int BPC, int SEAWEED_BPC, class string>
	class AlignmentPlotComputation  {
	public:
		typedef enum {
			LCS,
			BLCS,
			SEAWEEDS,
			SCORES,
			SCORESOVERLAP,
		} method_t;

		AlignmentPlotComputation() : 
			tp(NULL), c(NULL), output(NULL), input_is_preprocessed(false) {
		}

		~AlignmentPlotComputation() {
			if(c != NULL) {
				delete c;
				if(canrestart()) {
					std::string checkpointfile;
					pf.get("checkpointfile", checkpointfile);
					::remove(checkpointfile.c_str());
				}
			}
			if (tp) {
				delete tp;
			}
			if(output) {
				delete output;
			}
		}

		bool parsecommandline (int argc, char** argv) {
			using namespace std;
			using namespace utilities;
			namespace po = boost::program_options;

			// the x makes sure that 'N' characters
			// from the first string do not match 'N' characters
			// from the second string
			string firstsequencefile;
			string secondsequencefile;
			string outputidentifier;
			string profile1;
			string profile2;

			string method_s;

			string legalchars_a;
			string legalchars_b;
			int threads;
			int overlap;
			int max_windowpairs;


			po::positional_options_description posopts; 
			posopts.add("first-sequence", 1);
			posopts.add("second-sequence", 1);
			posopts.add("output-file", 1);
			posopts.add("profile1", 1);
			posopts.add("profile2", 1);

			po::options_description hidden("Hidden options");
			hidden.add_options()
				(	"first-sequence,f", 
					po::value< string >(&firstsequencefile)->default_value("first.txt"), 
					"Name of first input sequence")
				(	"second-sequence,s", 
					po::value< string >(&secondsequencefile)->default_value("second.txt"),
					"Name of second input sequence")
				(	"output-file,o", 
					po::value< string >(&outputidentifier)->default_value("result.txt"),
					"output file name")
				(	"profile1,x", 
					po::value< string >(&profile1)->default_value("profile1.txt"),
					"file name for first profile")
				(	"profile2,y", 
					po::value< string >(&profile2)->default_value("profile2.txt"),
					"file name for second profile")
			;

			po::options_description desc("Options");
			
			desc.add_options()
				(	"help,h", 
					"output help message")
				(	"checkpoint,c", 
					"enable checkpointing (resuming if checkpoint file exists for the job")
				(	"full-output,F", 
					"output all relevant window matches (output can become very large!)")
				(	"max-windowpairs,M", 
					po::value<int> (&max_windowpairs)->default_value(-1), 
					"specify the maximum number of windows that are reported.")
				(	"method,m", 
					po::value< string >(&method_s)->default_value("seaweeds"), 
					"choose method to use: [lcs|blcs|seaweeds|scores|scoresoverlap]" )
				(	"overlap,v", 
					po::value<int> (&overlap)->default_value(-1), 
					"specify overlap size for scoresoverlap method")
				(	"threads", 
					po::value<int> (&threads)->default_value(-1), 
					"specify number of threads to use (default = number of cores)")
				(	"legalchars_a,a", 
					po::value<string> (&legalchars_a)->default_value("ABGCTNxz"), 
					"specify input alphabet translation for first input, default: ABGCTNxz")
				(	"legalchars_b,b", 
					po::value<string> (&legalchars_b)->default_value("ABGCTxNz"), 
					"specify input alphabet translation for first input, default: ABGCTxNz")
			;

			po::options_description all_opts;
			all_opts.add(desc).add(hidden);

			po::variables_map vm;
			store(po::command_line_parser(argc, argv).
				options(all_opts).positional(posopts).run(), vm);
			notify(vm);

			if (vm.count("help")) {
				cout << desc;
				bsp_abort("\n");
			}

			// write result as we go. sorting can be done in postprocessing
			std::string checkpointfile = outputidentifier+"_progress";
			if(vm.count("checkpoint") && utilities::fexists(checkpointfile.c_str())) {
				pf.set("checkpoint", checkpointfile );
			}

			if (vm.count("full-output")) {
				pf.set("full-output", (int)1);
			} else {
				pf.set("full-output", (int)0);
			}

			int firststepwidth = 1;
			int secondstepwidth = 1;
			int windowlength = 100;
			double cutoffthreshold = 1.0;

			// read inputs
			ifstream f1;
			ifstream f2;

			f1.open(firstsequencefile.c_str());
			f2.open(secondsequencefile.c_str());

			// read input from files
			if (f1.bad() || f2.bad()) {
				bsp_abort("ERROR: could not read input files.");
			}
			f1 >> firststepwidth >> secondstepwidth >> windowlength >> cutoffthreshold >> a;
			f2 >> b;

			f1.close();
			f2.close();

			method_t method = SEAWEEDS;

			if(method_s == "lcs") {
				method = LCS;
			} else if(method_s == "blcs") {
				method = BLCS;
			} else if(method_s == "seaweeds") {
				method = SEAWEEDS;
			} else if(method_s == "scores") {
				method = SCORES;
			} else if(method_s == "scoresoverlap") {
				method = SCORESOVERLAP;
			} else {
				cout << "ERROR: Unknown method: " << method_s << endl;
				return false;
			}

			int method_int = (int)method;
			pf.set("method", method_int);
			pf.set("firststepwidth", firststepwidth);
			pf.set("secondstepwidth", secondstepwidth);
			pf.set("windowlength", windowlength);
			pf.set("cutoffthreshold", cutoffthreshold);
			pf.set("legalchars_a", legalchars_a);
			pf.set("legalchars_b", legalchars_b);
			pf.set("outputidentifier", outputidentifier);
			pf.set("overlap", overlap);
			pf.set("profile1", profile1);
			pf.set("profile2", profile2);
			pf.set("threads", threads);
			pf.set("max_windowpairs", max_windowpairs);

			return true;
		}

		void preprocess_inputs() {
			using namespace std;
			using namespace lcs;
			using namespace windowlcs;
			using namespace utilities;
			string legalchars_a("ABGCTNxz");
			string legalchars_b("ABGCTxNz");
			pf.get("legalchars_a", legalchars_a, legalchars_a);
			pf.get("legalchars_b", legalchars_b, legalchars_b);

			if (a.size() == 0 || b.size() == 0) {
				bsp_abort("One of the input strings is empty -- file empty or not found?\n");
			}

			translatestring(a, legalchars_a);
			translatestring(b, legalchars_b);

			a = ScoreTranslation<string>::translate(a);
			b = ScoreTranslation<string>::translate(b);
			input_is_preprocessed = true;
		}

		utilities::Checkpointable * generate_matcher() {
				using namespace std;
				using namespace lcs;
				using namespace windowlcs;
				using namespace utilities;
				int firststepwidth = 1;
				int secondstepwidth = 1;
				int windowlength = 100;
				double cutoffthreshold = 1.0; 

				pf.get("firststepwidth", firststepwidth, firststepwidth);
				pf.get("secondstepwidth", secondstepwidth, secondstepwidth);
				pf.get("windowlength", windowlength, windowlength);
				pf.get("cutoffthreshold", cutoffthreshold, cutoffthreshold);

				int m = (int) SEAWEEDS; 
				method_t method;

				pf.get("method", m, m);
				method = (method_t)m;

				utilities::Checkpointable * computation = NULL;

				switch(method) {
		case LCS: {
			//  This is a naive matcher using standard LCS score computation
			//  Running time is O(m n w^2)
			NaiveWindowWindowMatcherGenerator<string, Llcs<> > matcher;
			computation = matcher(a, b,
				ScoreTranslation<string>::translatecoord_fwd(windowlength),
				(size_t)ScoreTranslation<string>::translatescore_fwd(windowlength, windowlength, cutoffthreshold),
				ScoreTranslation<string>::translatecoord_fwd(firststepwidth),
				ScoreTranslation<string>::translatecoord_fwd(secondstepwidth),
				tp
				);
				  }
				  break;
		case BLCS:
			{
				_a = IntegerVector<BPC>(a.c_str()),
				_b = IntegerVector<BPC>(b.c_str());
				//  This is a naive matcher using faster bit-parallel LCS score computation
				//  Running time is O(m n w^2 / wordsize)
				NaiveCIPRWindowWindowMatcherGenerator<BPC> matcher;
				computation = matcher(_a, _b,
					ScoreTranslation<string>::translatecoord_fwd(windowlength),
					(size_t)ScoreTranslation<string>::translatescore_fwd(windowlength, windowlength, cutoffthreshold),
					ScoreTranslation<string>::translatecoord_fwd(firststepwidth),
					ScoreTranslation<string>::translatecoord_fwd(secondstepwidth),
					tp
					);
			}
			break;
		case SEAWEEDS:
			{
				//  This is an automaton-based matcher.
				//  Running time is O(m n w)
				if(windowlength*2 > pow(2.0, (double)SEAWEED_BPC) - 3) {
					cerr << "The requested window length is too large." << endl;
					exit(0);
				}
				SeaweedWindowWindowMatcherGenerator<BPC, SEAWEED_BPC> matcher;
				_a = IntegerVector<BPC>(a.c_str()),
				_b = IntegerVector<BPC>(b.c_str());
				computation = matcher(_a, _b,
					ScoreTranslation<string>::translatecoord_fwd(windowlength),
					(size_t)ScoreTranslation<string>::translatescore_fwd(windowlength, windowlength, cutoffthreshold),
					ScoreTranslation<string>::translatecoord_fwd(firststepwidth),
					ScoreTranslation<string>::translatecoord_fwd(secondstepwidth),
					tp
					);
			}
			break;
		case SCORES: {
			//  This is an automaton-based matcher.
			//  Running time is O(m n w)
			if(windowlength*2 > pow(2.0, (double)SEAWEED_BPC) - 3) {
				cerr << "The requested window length is too large." << endl;
				exit(0);
			}
			ScorematrixWindowWindowMatcherGenerator<BPC, SEAWEED_BPC> matcher;
			_a = IntegerVector<BPC>(a.c_str()),
			_b = IntegerVector<BPC>(b.c_str());
			computation = matcher (_a, _b,
				ScoreTranslation<string>::translatecoord_fwd(windowlength),
				(size_t)ScoreTranslation<string>::translatescore_fwd(windowlength, windowlength, cutoffthreshold),
				ScoreTranslation<string>::translatecoord_fwd(firststepwidth),
				ScoreTranslation<string>::translatecoord_fwd(secondstepwidth),
				tp
				);
					 }
					 break;
		case SCORESOVERLAP: {
			//  This is an automaton-based matcher.
			//  Running time is O(m n w)
			if(windowlength*2 > pow(2.0, (double)SEAWEED_BPC) - 3) {
				cerr << "The requested window length is too large." << endl;
				exit(0);
			}
			int overlap;
			pf.get("overlap", overlap, -1);
			SeaweedOverlapMatcherGenerator<BPC, SEAWEED_BPC> matcher;
			_a = IntegerVector<BPC>(a.c_str()),
			_b = IntegerVector<BPC>(b.c_str());
			computation = matcher (_a, _b,
				ScoreTranslation<string>::translatecoord_fwd(windowlength),
				(size_t)ScoreTranslation<string>::translatescore_fwd(windowlength, windowlength, cutoffthreshold),
				ScoreTranslation<string>::translatecoord_fwd(firststepwidth),
				ScoreTranslation<string>::translatecoord_fwd(secondstepwidth),
				tp,
				overlap
				);
					}
					break;
				}

				if(canrestart()) {
					Checkpointer * c = new Checkpointer();
					std::string checkpointfile;
					pf.get("checkpoint", checkpointfile);
					c->initialize(checkpointfile.c_str(), ParameterFile::BINARY);
					computation->set_checkpointer(c);
				}

				return computation;
		}

		/**
		 * @brief translate a string through alphabet translation 
		 */
		void translatestring(std::string & a, std::string chars = "AGCT") {
			for(size_t j = 0; j < a.size(); ++j) {
				char c = a[j];
				bool found = false;
				for (size_t k = 0; k < chars.size(); ++k) {
					if (c == chars[k]) {
						c = 'A' + (char)k;
						found = true;
						break;
					}
				}
				a[j] = c;
			}
		}

		/**
		 * @brief set a new output file name
		 */
		void setoutput(std::string const & outputidentifier, int offset = 0, int p_offset = 0) {
			if(tp != NULL) {
				delete tp;
				tp = NULL;
			}
			if (output != NULL)	{
				delete output;
				output = NULL;
			}
			pf.set("outputidentifier", outputidentifier);
			
			if (outputidentifier == "")	{
				return;
			}

			std::string checkpointfile = outputidentifier+"_progress";
			output = new std::ofstream;

			if(canrestart() && utilities::fexists(checkpointfile.c_str())) {
				pf.set("checkpoint", checkpointfile );
				output->open(outputidentifier.c_str(), std::ios::out | std::ios::app);
			} else {
				pf.remove("checkpoint");
				output->open(outputidentifier.c_str());
			}
		
			double cutoffthreshold = 0.0;
			int winlen = 100;
			int firststepwidth = 1;
			int secondstepwidth = 1;
			int full_output = 0;
			int max_windowpairs = -1;

			pf.get("firststepwidth", firststepwidth , firststepwidth );
			pf.get("secondstepwidth", secondstepwidth , secondstepwidth );
			pf.get("cutoffthreshold", cutoffthreshold, cutoffthreshold);
			pf.get("windowlength", winlen, winlen);
			pf.get("full-output", full_output, full_output);
			pf.get("max_windowpairs", max_windowpairs, max_windowpairs);

			if(full_output) {
				tp = new translate_and_print<string>(*output, winlen, winlen, cutoffthreshold, 
					get_profile_size_a(), 
					get_profile_size_b(), 
					firststepwidth, secondstepwidth);
			} else {
				// This is the formula from the original window alignment code.
				// We return at least 3000 window pairs, and we expect an additional
				// 0.4 window pairs per base. We use the average length of string 
				// a and string b as the number of bases.
				size_t buffersize = 3000 + 0.2 * (get_real_a_len() + get_real_b_len());
				if (max_windowpairs > 0) {
					buffersize = max_windowpairs;
				}
				cout << "Window buffer size is " << buffersize << endl;
				tp = new translate_and_print_with_buffer<string>(*output, 
					buffersize,
					winlen, winlen, cutoffthreshold, 
					get_profile_size_a(), 
					get_profile_size_b(), 
					firststepwidth, secondstepwidth);
			}
			tp->offset = offset;
			tp->p_offset = p_offset;
			if (!canrestart()) {
				*output << cutoffthreshold << std::endl;
			}
		}

		/**
		 * @brief copy our settings to another computation
		 * This also resets the output settings for computation p.
		 */
		void assign_settings(AlignmentPlotComputation & p) {
			p.pf = pf;
			p.c = NULL;
			if (p.tp != NULL) {
				delete p.tp;
				p.tp = NULL;
			}
			if (p.output != NULL) {
				delete p.output;
				p.output = NULL;
			}
		}


		/**
		 * @brief get name for the output of this computation (specified in the options)
		 */
		string getoutputidentifier() {
			std::string outputidentifier;
			pf.get("outputidentifier", outputidentifier);
			return outputidentifier;
		}

		/**
		 * @return true if this computation will restart from a checkpoint file
		 */
		bool canrestart () {
			return pf.hasValue("checkpoint");
		}

		/**
		 * @return the parameters for this computation 
		 */
		utilities::ParameterFile & parameters() {
			return pf;
		}

		/**
		 * @return the first input string
		 */
		string & get_a() {
			return a;
		}

		/**
		 * @return the first input string
		 */
		string & get_b() {
			return b;
		}

		/**
		 * @brief get length of input a before blowup
		 */
		size_t get_real_a_len() {
			return input_is_preprocessed ? lcs::ScoreTranslation<string>::translatecoord_bk(a.size()) :
				a.size();
		}

		/**
		 * @brief get length of input b before blowup
		 */
		size_t get_real_b_len() {
			return input_is_preprocessed ? lcs::ScoreTranslation<string>::translatecoord_bk(b.size()) :
				b.size();
		}

		/**
		 * @brief write match profile for string a
		 */
		const double * get_profile_a() {
			return tp->profile_a;
		}

		/**
		 * @brief write match profile for string b
		 */
		const double * get_profile_b() {
			return tp->profile_b;
		}

		/**
		 * @brief get the size of profile a
		 */
		size_t get_profile_size_a() {
			int winlen = 100;
			int firststepwidth = 1;

			pf.get("firststepwidth", firststepwidth , firststepwidth );
			pf.get("windowlength", winlen, winlen);
			return ICD(get_real_a_len()-winlen+1, firststepwidth);
		}

		/**
		 * @brief get the size of profile b
		 */
		size_t get_profile_size_b() {
			int winlen = 100;
			int secondstepwidth = 1;
			pf.get("secondstepwidth", secondstepwidth , secondstepwidth );
			pf.get("windowlength", winlen, winlen);
			return ICD(get_real_b_len()-winlen+1, secondstepwidth);
		}

		private:
			string  a;	///< the original first input
			string  b;  ///< the original second input
			utilities::ParameterFile pf; ///< all parameters describing this computation
			utilities::Checkpointer * c; ///< checkpointer (if we can use one)
			std::ofstream * output; ///< the output file
			bool input_is_preprocessed; ///< true if input has been blown up according to scores

			translate_and_print<string> * tp; ///< the output printing helper.
			utilities::IntegerVector<BPC> _a, _b; ///< the translated inputs
	};

};

#endif // windowwindow_factory_h__
