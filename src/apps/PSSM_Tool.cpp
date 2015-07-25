/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "bsp_cpp/bsp_cpp.h"

#include "PSSM/PSSM_Tool.h"


/** The PSSM Tool works on a set of PSSMs, which are stored here */
std::vector < pssm > g_pssms;

/** The PSSM Tool automatically reads or creates a background model */
pssmsequencemodel g_background;
std::string g_background_id;

/** from ParameterFile.h **/
namespace utilities {
	extern std::string trimstring(const std::string & g, char c);
};

/************************************************************************/
/* Main part                                                            */
/************************************************************************/

int main (int argc, char ** argv) {
	utilities::init_xasmlib();
	bsp_init(&argc, &argv);

	std::string helptext;

	using namespace std;
	using namespace bsp;

	try {
		namespace po = boost::program_options;
		po::options_description desc("Generic Options");

		typedef std::map<std::string, boost::shared_ptr<App> > actionmap;

		actionmap actions;
		actions["distances"] = boost::make_shared<PSSM_DistancesApp>();
		actions["profile"] = boost::make_shared<PSSM_ProfileApp>();
		actions["comparescores"] = boost::make_shared<PSSM_CompareScoresApp>();
		actions["score"] = boost::make_shared<PSSM_ScoreApp>();
		actions["info"] = boost::make_shared<PSSM_InfoApp>();
		actions["export"] = boost::make_shared<PSSM_ExportApp>();

		string acts = "Available actions: "; 
		for (actionmap::iterator it = actions.begin(); it != actions.end(); ++it) {
			acts+=it->first + " ";
		}
		
		desc.add_options()
			("help,h",
			"output help message")
			("action,a", po::value<string>()->required(), acts.c_str())
			("modelfile,m", po::value<string>(), 
			"Specify a sequence background model input file (optional)")
			("pssmfile,f", po::value<string>()->required(), 
			"Specify an input file name containing pssms in JSON or JASPAR text format")
			;

		po::options_description all_opts;
		all_opts.add(desc);

		for (actionmap::iterator it = actions.begin(); it != actions.end(); ++it) {
			it->second->add_opts(all_opts);
		}

		{
			ostringstream oss;
			oss << all_opts;
			helptext = oss.str();
		}

		po::variables_map vm;
		
		bsp_command_line(argc, argv, all_opts, vm);

		if (vm.count("help")) {
			cout << helptext;
			bsp_end();
			exit (0);
		}
		
		string pssmfile = vm["pssmfile"].as<string>();
		
		// TODO: move this to apps and create dependencies
		// read in pssms
		ifstream pf (pssmfile.c_str());

		if (pssmfile.size() >= 6 && 0 == pssmfile.compare(pssmfile.size() - 6, 6, ".mjson")) {
			string line;

			while ( getline(pf, line, "\n") ) {
				utilities::trimstring(line, ' ');
				utilities::trimstring(line, '\t');
				utilities::trimstring(line, '\n');

				if (line == "")
				{
					continue;
				}

				istringstream i (line);
				pssms::PSSM<> p;
				i >> p;
				g_pssms.push_back(p);
				line = "";
			}
		} else if (pssmfile.size() >= 5 && 0 == pssmfile.compare(pssmfile.size() - 5, 5, ".json")) {
			pssms::PSSM<> p;
			pf >> p;
			g_pssms.resize(1);
			g_pssms[0] = p;
		} else if (pssmfile.size() >= 4 && 0 == pssmfile.compare(pssmfile.size() - 4, 4, ".txt")) {
			try
			{
				for (;;)
				{
					pssms::PSSM<> p;
					p.load_jaspar_pssm(pf);
					g_pssms.push_back(p);
				}
			} catch (std::runtime_error e) {
				if(g_pssms.size() == 0) {
					throw e;
				}
			}
		} else {
			throw runtime_error("Unknown input file type (allowed: mjson / json / txt )");
		}

		cerr << "Read " << g_pssms.size() << " PSSMS" << endl;

		// read in sequence model
		g_background_id = "default";
		if (vm.count("modelfile") > 0) {
			string modelfile = vm["modelfile"].as<string>();

			pssmsequencemodel mm (new pssmmarkovmodel());

			if ( utilities::fexists(modelfile.c_str()) ) {
				cerr << "Reading model from " << modelfile << endl;
				ifstream in(modelfile.c_str());
				in >> (*mm);
				g_background_id = modelfile;
				for (size_t j = 0; j < g_background_id.size(); ++j) {
					int c = tolower(g_background_id[j]);
					if( (c >='0' && c <= '9') || 
						(c >='a' && c <= 'z') ) {
						g_background_id[j] = c;
					} else {
						g_background_id[j] = '_';
					}
				}
			} else {
				throw runtime_error("Input model file not found");
			}
			g_background = mm;
		} else {
			pssmsequencemodel mm (new pssmmarkovmodel());
			g_background = mm;
		}

		actionmap::iterator action = actions.find(vm["action"].as<string>());

		if (action == actions.end()) {
			throw std::runtime_error("Unknown action.");
		} else {
			action->second->run(vm);
		}

	} catch (std::runtime_error e) {
		cerr << e.what() << endl;
		cerr << helptext << endl;
	} catch (std::exception e) {
		cerr << helptext << endl;
	}

	bsp_end();

	return EXIT_SUCCESS;
}
