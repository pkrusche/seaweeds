/***************************************************************************
*   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
*   peter@dcs.warwick.ac.uk                                               *
***************************************************************************/

#include "bspcpp_config.h"

#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <tbb/task_scheduler_init.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include "bspcpp/tools/utilities.h"
#include "bspcpp/tools/spawn.h"


class MultiWarmup {
public:
	void operator()(tbb::blocked_range<int> const & range) const {
		utilities::warmup(.01);
	}
};

using namespace std;

int main(int argc, char * argv[]) {

	if(argc < 2) {
		cerr << "Usage: timer [-w [seconds]] command line" << endl;
		return 0;
	}
	
	double wtime = 0.0;
	int offset = 1;
	
	if(strcmp(argv[1], "-w") == 0) {
		if(argc > 3) {
			wtime = atof(argv[2]);
			if(wtime > 0) {
				offset = 3;
			} else {
				wtime = 2.0;
				offset = 2;
			}
		}
		tbb::task_scheduler_init init;

		tbb::parallel_for<tbb::blocked_range<int>, 
			MultiWarmup>(
			tbb::blocked_range<int> (0, (int)wtime*1000), 
			MultiWarmup(),
			tbb::auto_partitioner()
		);

		utilities::warmup(wtime);
	}

	cout << "Executing " ;
	char ** new_argv = new char*[argc];
	for(int j = offset; j < argc; ++j) {
		new_argv[j-offset] = argv[j];
		cout << argv[j] << " " ;
	}
	cout << "..";

	new_argv[argc-1] = NULL;

	cout << "..";
	cout << endl;

	double t0 = utilities::time();

	_spawnvp(P_WAIT, argv[offset], new_argv);

	cout << endl << "Time elapsed: " << utilities::time() - t0 << " s " << endl;

	return 0;
}
