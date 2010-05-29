/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
/*
#define _VERBOSETEST_WINDOWLCS_BOASSON
#define _VERBOSETEST_WINDOWLCS_CIPR
#define _VERBOSETEST
#define _VERBOSETEST2
#define _VERBOSETEST3
*/

//#define _DEBUG_SEAWEEDS
#define _SEAWEEDS_VERIFY

#include "../autoconfig.h"
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

#include "tests/Testing.h"

using namespace std;
using namespace utilities;
using namespace lcs;
using namespace windowlocal;
using namespace tuning;

int patsize = 5;
int winlen = 5*patsize;

template <int _bpc>
class tworandomstrings {
public:
	IntegerVector<_bpc> text;
	IntegerVector<_bpc> pattern;

	tworandomstrings(int textsize) : text(textsize), pattern(patsize) {
        using namespace utilities;
        const long bitmask = (1 << _bpc) - 1; // 0x3; // 

		for(size_t j = 0; j < pattern.size(); ++j) {
			pattern[j] = rand() & bitmask;
		}

		for(size_t j = 0; j < text.size(); ++j) {
			text[j] = rand() & bitmask;
		}
#ifdef _VERBOSETEST
		cout << "text: " << text << endl;
		cout << "pat:  " << pattern << endl;
#endif
	}
};

template<
    class _counter,
    int _bpc
>
class WindowLocalLCSTest {
public:
	size_t operator() (tworandomstrings<_bpc> rs) {

		_counter windowcounter(winlen, rs.pattern);
		windowlocal::report_nothing<> rn;
		return windowcounter(rs.text, &rn);
    }
};

#define BITSPERCHAR    8
#define BOASSON_OMEGA  16

typedef utilities::IntegerVector<BITSPERCHAR> bit_string;

typedef WindowLocalLCSTest<
    windowlocal::NaiveWindowLocalLlcs<lcs::Llcs< bit_string >, bit_string  >, 
	BITSPERCHAR
> LlcsTest;

typedef WindowLocalLCSTest<
    windowlocal::NaiveWindowLocalLlcs<lcs::LlcsCIPR<BITSPERCHAR>, 
	bit_string >, 
	BITSPERCHAR
> LlcsCIPRTest;

typedef WindowLocalLCSTest<
	windowlocal::NaiveBitparallelWindowLocalLlcs<BITSPERCHAR>, 
	BITSPERCHAR
> LlcsCIPRTest2;

typedef WindowLocalLCSTest<
	windowlocal::BoassonMPRAMMatcher<BITSPERCHAR, windowlocal::report_nothing<>, BOASSON_OMEGA>, 
	BITSPERCHAR
> BoassonTest;
typedef WindowLocalLCSTest<
windowlocal::SeaweedWindowLocalLlcs<BITSPERCHAR, windowlocal::report_nothing<>, BOASSON_OMEGA>, 
BITSPERCHAR
> SeaweedTest;

typedef WindowLocalLCSTest<
windowlocal::ScoreMatrixWindowLocalLlcs < windowlocal::report_nothing<>, 
	seaweeds::ScoreMatrix< 
		seaweeds::ImplicitStorage<
			seaweeds::Seaweeds<
				BOASSON_OMEGA, BITSPERCHAR
			>
	> > >,
BITSPERCHAR
> ScorematrixTest;

typedef LOKI_TYPELIST_6(LlcsTest, LlcsCIPRTest, LlcsCIPRTest2, BoassonTest, SeaweedTest, ScorematrixTest) algorithms;
//typedef LOKI_TYPELIST_1(ScorematrixTest) algorithms;

int main(int argc, char *argv[]) {
	int add = 6*patsize;
	int NUM = 100;
	int inc = 1;

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

	if(argc > 5) {
		winlen = atoi(argv[5]);
	} else {
		winlen = 5*patsize;
	}

	TestIterator < 
		size_t, 
		tworandomstrings<BITSPERCHAR>,
		algorithms
	> t;
	t(add, NUM, inc);

	return EXIT_SUCCESS;
}

