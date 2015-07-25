/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef POSTPROCESSWINDOWS_H_
#define POSTPROCESSWINDOWS_H_

#include "../App.h"

class WindowPostProcess_App : public App {
public:
	
	void add_opts( boost::program_options::options_description & all_opts ) {
		using namespace std;
		namespace po = boost::program_options;
		po::options_description opts("Alignment Plot Postprocessing Options");
		opts.add_options()
// from Alignmentplot_App : output-file, processors, cutoff
			("max-windows", po::value<int>()->default_value(10000), 
			"Maximum number of window-pairs to output (default: 10000)")
		;
		all_opts.add(opts);
	}

	void run( boost::program_options::variables_map & vm );
};

#endif /* POSTPROCESSWINDOWS_H_ */
