/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#define _VERBOSETEST

#include "../pk_config.h"

#include <iostream>
#include <loki/Typelist.h>

#include "bspcpp/tools/utilities.h"
#include "xasmlib/IntegerVector.h"
#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"
#include "seaweeds/ScoreMatrix.h"

#include "windowlocal/naive.h"
#include "windowlocal/naive_cipr.h"
#include "windowlocal/boasson.h"
#include "windowlocal/seaweeds.h"
#include "windowlocal/scorematrix.h"

#include "tuning/Timing.h"

using namespace std;
using namespace utilities;
using namespace lcs;
using namespace windowlocal;
using namespace tuning;

template<
    class _counter,
    int _bpc
>
class WindowLocalLCSBenchmark {
public:
	WindowLocalLCSBenchmark(size_t patternsize) : pattern(patternsize) {
        const long bitmask = (1 << _bpc) - 1;
		for(size_t j = 0; j < pattern.size(); ++j) {
			pattern[j] = rand() & bitmask;
		}
		windowcounter = new _counter(5*pattern.size(), pattern);
	}
	
	~WindowLocalLCSBenchmark() {
		delete windowcounter;
	}

    double operator() (size_t textsize) {
        using namespace utilities;
        const long bitmask = (1 << _bpc) - 1;
		IntegerVector<_bpc> text(textsize);

		for(size_t j = 0; j < text.size(); ++j) {
			text[j] = rand() & bitmask;
		}
	        
		double t0 = utilities::time();
		size_t count = (*windowcounter)(text);
		double t = utilities::time();
        
        return t - t0;
    }

private:
	IntegerVector<_bpc> pattern;
	_counter * windowcounter;
};

#define BITSPERCHAR 8

typedef tuning::Timing <
    WindowLocalLCSBenchmark<
	windowlocal::NaiveWindowLocalLlcs< lcs::Llcs<utilities::IntegerVector<BITSPERCHAR> >, utilities::IntegerVector<BITSPERCHAR> >, BITSPERCHAR >, 
        size_t > LlcsBenchmark;
typedef tuning::Timing <
    WindowLocalLCSBenchmark<
		windowlocal::NaiveBitparallelWindowLocalLlcs<BITSPERCHAR>, BITSPERCHAR >, 
        size_t > LlcsCIPR_with_preprocessing_Benchmark;
typedef tuning::Timing <
    WindowLocalLCSBenchmark<
        windowlocal::BoassonMPRAMMatcher<BITSPERCHAR>, BITSPERCHAR >, 
        size_t > BoassonBenchmark;
typedef tuning::Timing <
	WindowLocalLCSBenchmark<
		windowlocal::SeaweedWindowLocalLlcs<BITSPERCHAR, windowlocal::report_nothing<>, BITSPERCHAR>, BITSPERCHAR >, 
		size_t > SeaweedBenchmark;

typedef tuning::Timing <
	WindowLocalLCSBenchmark<
	windowlocal::ScoreMatrixWindowLocalLlcs < 
		windowlocal::report_nothing<>, 
		seaweeds::ScoreMatrix< 
			seaweeds::ImplicitStorage<
				seaweeds::Seaweeds<
					8, BITSPERCHAR
				>
		> > >,
	BITSPERCHAR >, 
		size_t > ScorematrixBenchmark;

typedef LOKI_TYPELIST_5(LlcsBenchmark, LlcsCIPR_with_preprocessing_Benchmark, BoassonBenchmark, SeaweedBenchmark, ScorematrixBenchmark) 
	algorithms;

int main(int argc, char *argv[]) {
	int add = 1000;
	int NUM = 20;
	int inc = 1000;
	int patsize = 15;

	init_xasmlib();

	if(argc > 1) {
		add = atoi(argv[1]);
	}

	if(argc > 2) {
		NUM = atoi(argv[2]);
	}

	if(argc > 3) {
		inc = atoi(argv[3]);
	}

	if(argc > 4) {
		patsize = atoi(argv[4]);
	}

	TimingList tdata;        
	MultiTiming < 
		size_t, 
		algorithms
	>::getTimings(add, NUM, inc, tdata, patsize);

	cout << tdata << endl;

	return EXIT_SUCCESS;
}
