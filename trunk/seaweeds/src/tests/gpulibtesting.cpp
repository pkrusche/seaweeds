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

typedef seaweeds::ScoreMatrix<int, seaweeds::ImplicitStorage<seaweeds::Seaweeds<32> > > SCOREMATRIX;

int main(int argc, char *argv[]) {
	using namespace std;
	utilities::init_xasmlib();
	int m = 10000;
	int n = 10000;
	const int K = 1;
	
	char * x = new char [m];
	char * y = new char [n];

	for (int j = 0; j < m-1; ++j) {
		x[j]='A'+(j+rand())%26;
	}

	for (int j = 0; j < n-1; ++j) {
		y[j]='A'+(rand()%26);
	}

	x[m-1] = 0;
	y[n-1] = 0;
	m--;
	n--;
	

	double t0 = utilities::time();

	SCOREMATRIX * sms[K];
	for (int k = 0; k < K; ++k) {
		sms[k] = new SCOREMATRIX(m,n);
		sms[k]->semilocallcs(x, y);
	}

	for (int k = 0; k < K; ++k) {
		delete sms[k];
	}

	double t1 = utilities::time();
	int * p = new int [m+n];
	for (int j = 0; j < m+n; ++j) {
		p[j] = j;
	}	
	GPUSeaweeds * gs[K];

	double _t1 = utilities::time();
	for (int k = 0; k < K; ++k) {
		gs[k] = new GPUSeaweeds();
		gs[k]->run(x, y, m, n, p);
	}

	for (int k = 0; k < K; ++k) {
		gs[k]->finish(p);
		delete gs[k];
	}

	delete [] x;
	delete [] y;
	delete [] p;

	double t2 = utilities::time();

	cout << "CPU Time: " << t1-t0 << endl; 
	cout << "GPU Time: " << t2-_t1 << endl; 

	return EXIT_SUCCESS;
}
