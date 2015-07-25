/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cmath>

#include <tbb/task_scheduler_init.h>
#include <tbb/mutex.h>

#include <boost/program_options.hpp>

#include "bsp_tools/utilities.h"

#include "datamodel/SequenceTranslation.h"
#include "sequencemodel/MarkovModel.h"

int main (int argc, char ** argv) {
	utilities::init_xasmlib();
	using namespace std;
	namespace po = boost::program_options;
	po::options_description desc("Options");

	try {


		desc.add_options()
			(	"help,h",
			"output help message")
			("action,a", po::value<string>()->required(), 
			"Specify the action to perform: learn|generate")
			("modeltype,m", po::value<string>()->default_value("markov3"), 
			"the model type: markov1|markov2|...|markov10, default is markov4")
			("modelfile,f", po::value<string>(), 
			"the filename for reading/writing the model data")
			("length,l", po::value<int>()->default_value(0), 
			"The length of sequence to generate")
			("sequence,s", po::value<string>(), 
			"the input sequence file name for learning a model")
			( "newmodel,n",
			"do not read model input file, but rather create a new model")
			;

		po::options_description all_opts;
		all_opts.add(desc);

		po::variables_map vm;
		store(po::command_line_parser(argc, argv).
			options(all_opts).run(), vm);
		notify(vm);

    if (vm.count("help")) {
        cerr << desc;
		exit (0);
    }

	string action = vm["action"].as<string> ();
	string modeltype = vm["modeltype"].as<string> ();
	string modelfile = "no model input file";
	int length = vm["length"].as<int> ();
	
	sequencemodel::SequenceModel< utilities::IntegerVector<2> > * model;

	if (modeltype.find("markov") == 0) {
		int order = 3;
		if (modeltype.size() > 6) {
			order = modeltype.at(6) - '0';
			if (order > 0 && order < 20) {
				cerr << "Using Markov model of order " << order << endl;
			} else {
				cerr << "Invalid model order: " << order << " " << modeltype << endl;
				exit (1);
			}
			model = new sequencemodel::MarkovModel<2, 2>;
			((sequencemodel::MarkovModel<>*)model)->order(order);
			
			if (vm.count("modelfile") > 0 && vm.count("newmodel") == 0) {
				modelfile = vm["modelfile"].as<string> ();
				if ( utilities::fexists(modelfile.c_str()) ) {
					cerr << "Reading model from " << modelfile << endl;
					ifstream in(modelfile.c_str());
					in >> (*model);
				} else {
					delete model;
					cerr << "Model file could not be opened.";
					exit (1);
				}
			}
		}
	} else {
		cerr << "Unknown model type." << endl;
		exit(1);
	}

	if (action == "learn") {
		if (vm.count("sequence") > 0) {
			string sequencefile = vm["sequence"].as<string> ();
			ifstream seqin (sequencefile.c_str());
			model->learn(seqin);
		} else {
			cerr << "no input sequence found for learning from." << endl;
			exit (1);
		}

		if (vm.count("modelfile") > 0) {
			modelfile = vm["modelfile"].as<string> ();
			ofstream of (modelfile.c_str());
			of << (*model);
		} else {
			cout << (*model);
		}
	} else if (action == "generate") {
		cout << ">" << modeltype << " (" << modelfile << "): " << length << " random bp" << endl;
		while ( length > 0 ) {
			int gen_len = min (length, 73);
			cout << datamodel::unwrap_sequence<2> ( model->generate_sequence( gen_len ) );
			length-= gen_len;
		}

	} else if (action == "dump") {
		model->dump(cout);
	} else {
		cerr << "Unknown action specified." << endl;
	}

	delete model;
	} catch (std::exception e) {
		cerr << e.what() << endl;
		cerr << desc << endl;
	}

	return EXIT_SUCCESS;
}
