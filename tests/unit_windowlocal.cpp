/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
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

#include "autoconfig.h"

#include <iostream>
#include <string.h>
#include <cstdlib>

#include "UnitTest++.h"
using namespace UnitTest;

#include <bsp_tools/utilities.h>


#include "util/TypeList.h"
#include "xasmlib/IntegerVector.h"
#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"

#include "seaweeds/ScoreMatrix.h"

#include "windowlocal/naive.h"
#include "windowlocal/naive_cipr.h"
#include "windowlocal/boasson.h"
#include "windowlocal/seaweeds.h"
#include "windowlocal/scorematrix.h"

#include "Testing.h"

using namespace std;
using namespace utilities;
using namespace lcs;
using namespace windowlocal;
using namespace tuning;

#define patsize 5
#define winlen  (5*patsize)

template <int _bpc>
class tworandomstrings {
public:
	IntegerVector<_bpc> text;
	IntegerVector<_bpc> pattern;

	tworandomstrings(size_t textsize) : text(textsize), pattern(patsize) {
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
		_counter * windowcounter = new _counter(winlen, rs.pattern);
		size_t j = windowcounter->count(rs.text);
		delete windowcounter;
		return j;
    }
};

#define BITSPERCHAR    8
#define BOASSON_OMEGA  9

typedef utilities::IntegerVector<BITSPERCHAR> bit_string;

typedef WindowLocalLCSTest<
    windowlocal::WindowLocalLCS< lcs::Llcs< bit_string > >, 
	BITSPERCHAR
> LlcsTest;

typedef WindowLocalLCSTest<
    windowlocal::WindowLocalLCS<lcs::LlcsCIPR<BITSPERCHAR> >, 
	BITSPERCHAR
> LlcsCIPRTest;

typedef WindowLocalLCSTest<
	windowlocal::BPWindowLocalLCS<BITSPERCHAR>, 
	BITSPERCHAR
> LlcsCIPRTest2;

typedef WindowLocalLCSTest<
	windowlocal::BoassonMPRAMMatcher<BITSPERCHAR, BOASSON_OMEGA>, 
	BITSPERCHAR
> BoassonTest;

typedef WindowLocalLCSTest<
windowlocal::SeaweedWindowLocalLCS<BITSPERCHAR, BOASSON_OMEGA>, 
BITSPERCHAR
> SeaweedTest;

typedef WindowLocalLCSTest<
windowlocal::ScoreMatrixWindowLocalLCS < 
	seaweeds::ScoreMatrix< 
		seaweeds::ImplicitStorage<
			seaweeds::Seaweeds<
				BOASSON_OMEGA, BITSPERCHAR
			>
	> > >,
BITSPERCHAR
> ScorematrixTest;

typedef TYPELIST_6(LlcsTest, LlcsCIPRTest, LlcsCIPRTest2, BoassonTest, SeaweedTest, ScorematrixTest) algorithms;

namespace {
	TEST(Test_Windowlocal_LCS)
	{
		int add = 6*patsize;
		int NUM = 20;
		int inc = 1;

		init_xasmlib();

		TestIterator < 
			size_t, 
			tworandomstrings<BITSPERCHAR>,
			algorithms
		> t;
		t(add, NUM, inc);
	}
};
