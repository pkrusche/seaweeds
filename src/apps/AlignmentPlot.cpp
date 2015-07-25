/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "bsp_cpp/bsp_cpp.h"

#include "Seaweeds/AlignmentPlot_App.h"

#include "xasmlib/xasmlib.h"
#include "global_options.h"

/************************************************************************/
/* Main part                                                            */
/************************************************************************/

int main (int argc, char ** argv) {
	bsp_init(&argc, &argv);
	utilities::init_xasmlib();

	// read global parameter files.
	bsp::bspcpp_read_global_options();

	std::string helptext;

	using namespace std;
	using namespace bsp;

	try {
		namespace po = boost::program_options;
		po::options_description desc("Generic Options");

		// calculate number of processors
		int num_threads = 1;

		global_options.get("threads", num_threads, -1);
		if (num_threads <= 0 ) {
			num_threads = tbb::task_scheduler_init::default_num_threads();
		}

		// use nprocs*threads virtual processors
		int processors = bsp_nprocs() * num_threads;

		desc.add_options()
			("help,h",
			"output help message")
			("processors,p", po::value<int>()->default_value(processors), 
			"Number of processors to use for parallel computations")
			;

		po::options_description all_opts;
		all_opts.add(desc);

		AlignmentPlot_App ap_app;

		ap_app.add_opts(all_opts);

		/* make help text */
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
		
		ap_app.run(vm);

	} catch (std::runtime_error e) {
		cerr << e.what() << endl;
		bsp_abort(helptext.c_str());
	} catch (std::exception e) {
		cerr << "An unknown error has occurred.";
		bsp_abort(helptext.c_str());
	}

	bsp_end();

	return EXIT_SUCCESS;
}
