/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include "config.h"

#include "bspcpp/CommandLine.h"

#include "windowwindow/postprocesswindows.h"

int main(int argc, const char* argv[] ) {
	using namespace std;
	using namespace utilities;

	CommandLine cmd(argc, argv);

	if(argc < 2) {
		cout << "Usage: " << cmd.getExecutable() << " [resultfilename1] [resultfilename2] ...  options" << endl
			 << endl
			 << "Options:" << endl
			 << "   -m [number]  -> maximum number of windows to export" << endl
			 << "   -o [filename]-> output file name" << endl;

		return 0;
	}

	postprocess::postprocess(argc, argv);

	return EXIT_SUCCESS;
}
