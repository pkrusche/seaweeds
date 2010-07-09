/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/


//#define _VERBOSETEST
//#define _DEBUG_SEAWEEDS

#include "autoconfig.h"

#include <iostream>

#include "gpulib/ATI_Gpulib.h"
#include "bspcpp/tools/utilities.h"

#include "seaweeds/ScoreMatrix.h"

using namespace seaweeds;
using namespace utilities;

typedef ScoreMatrix<seaweeds::ImplicitStorage<seaweeds::Seaweeds<16> > > SCOREMATRIX;
typedef IntegerVector<8> string_t;

int main(int argc, char *argv[]) {
	using namespace std;
	utilities::init_xasmlib();
	int m = 200;
	int n = 10000;
	const int K = 20;
	
	string_t x [K];
	string_t y (n);

	for(int k = 0; k < K; ++k) {
		x[k].resize(m);
		for (int j = 0; j < m; ++j) {
			x[k][j]='A'+(j+rand())%26;
		}
	}

	for (int j = 0; j < n; ++j) {
		y[j]='A'+(rand()%26);
	}
	
	double t0 = utilities::time();
	
	SCOREMATRIX * sms[K];
	for (int k = 0; k < K; ++k) {
		sms[k] = new SCOREMATRIX(m,n);
		sms[k]->semilocallcs(x[k], y);
	}

	double t1 = utilities::time();
	MultiSeaweeds_GPU * gs;

	double _t1 = utilities::time();
	gs = new MultiSeaweeds_GPU();
	gs->run(x, K, y);
	double t2 = utilities::time();

	cout << "Results:" << endl;
	for(int k = 0; k < K; ++k) {
		for (int j = 0; j < m+n; ++j) {
			int cpu = sms[k]->get_archive()[j];
			int gpu = gs->get_seaweedpermutation(k)[j];
			if (cpu != gpu) {
				cout << "MISMATCH! " << endl;
				cout << "CPU, " << k << "," << j << ": " << cpu << endl;;
				cout << "GPU, " << k << "," << j << ": " << gpu << endl;
			}
		}
		
	}
	
	delete gs;
	for (int k = 0; k < K; ++k) {
		delete sms[k];
	}


	cout << "CPU Time: " << t1-t0 << endl; 
	cout << "GPU Time: " << t2-_t1 << endl; 

	return EXIT_SUCCESS;
}
