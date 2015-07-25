/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __AlignmentPlot_H__
#define __AlignmentPlot_H__

#include "../App.h"

/**
 * Alignment plot app
 */
class AlignmentPlot_App : public App  {
public:
	void add_opts( boost::program_options::options_description & all_opts ) {
		using namespace std;
		namespace po = boost::program_options;

		po::positional_options_description posopts;
		posopts.add("first-sequence", 1);
		posopts.add("second-sequence", 1);
		posopts.add("output-file", 1);
		posopts.add("profile1", 1);
		posopts.add("profile2", 1);

		po::options_description hidden("Input/Output Options");
		hidden.add_options()
			(	"first-sequence",
				po::value< string >() -> default_value("first.txt"),
				"Name of first input sequence")
			(	"second-sequence",
				po::value< string >()-> default_value("second.txt"),
				"Name of second input sequence")
			(	"output-file",
				po::value< string >()-> default_value("result.txt"),
				"output file name")
		;
		po::options_description desc("Alignment Plot Options");
		desc.add_options()
			(	"full-output,F",
				"output all relevant window matches (output can become very large!)")
			(	"verbose,V",
				"Verbose output.")
			(	"windowsize,w",
				po::value<int>()-> default_value(100),
				"The window length parameter for the alignment plot. Default is 100.")
			(	"cutoff,T",
				po::value<double>()-> default_value(1.0),
				"The minimum window score required to report a window. Default is 1.0.")
			(	"method,m",
				po::value< string >()-> default_value("seaweeds"),
				"choose method to use: [blcs|seaweeds|nw] (default: seaweeds)" )
		;

		all_opts.add(desc).add(hidden);
	}

	void run( boost::program_options::variables_map & vm );
};

#endif /** __AlignmentPlot_H__ */
