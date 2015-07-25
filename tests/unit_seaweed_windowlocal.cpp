/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

// #define _VERBOSETEST_WINDOWLCS
#define _SEAWEEDS_VERIFY
#define _SEAWEEDS_VERIFY_THROW

#include "autoconfig.h"

#include <iostream>

#include "UnitTest++.h"
using namespace UnitTest;

#include <bsp_tools/utilities.h>

#include "windowlocal/seaweeds.h"

#define BPC 8

using namespace std;
using namespace utilities;
using namespace lcs;
using namespace windowlocal;

typedef SeaweedWindowLocalLCS<BPC, BPC> Seaweeds;

void test_wllcs (size_t tlen, size_t plen, size_t windowlen, size_t grid = 1, int k = 1 ) {
	Seaweeds :: string text(tlen);
	Seaweeds :: string pattern(plen);

	for (int i = 0; i < k; ++i) {
		cout << "l = " << tlen << " p = " << plen << " w = " << windowlen << " g = " << grid << endl;
		for(size_t j = 0; j < pattern.size(); ++j) {
			pattern[j] = rand() & 3; //(Seaweeds::string::msb + Seaweeds::string::lsbs);
		}

		for(size_t j = 0; j < text.size(); ++j) {
			text[j] = rand() & 3; //(Seaweeds::string::msb + Seaweeds::string::lsbs);
		}

		Seaweeds sw(windowlen, pattern, grid);
		sw.count(text);
	}
}

namespace {
	class wr : public windowlocal::window_reporter {
	public:
		wr() {
			for (int i = 0; i < 100; ++i) {
				scores[i] = -1;
			}
		}

		void report_score(windowlocal::window const & w) {
			CHECK(w.x1 == 5);
			CHECK(w.x0 >=0 && w.x1 < 100);

			scores[w.x0] = w.score;
		}

		void checkme(double val) {
			for (int i = 0; i < 100; ++i) {
				CHECK_CLOSE(val, scores[i], 0.001);
			}
		}

		double scores[100];
	};

	TEST(Test_Seaweeds_WindowlocalLCS_Allmatch) {
		Seaweeds::string t(109);
		Seaweeds::string p(10);

		for (int i = 0; i < t.size(); ++i)	{
			t[i] = 0;
		}
		for (int i = 0; i < p.size(); ++i)	{
			p[i] = 0;
		}

		wr w;
		Seaweeds sw(10, p, 1);
		sw.count(t, &w, 0, 5);		
		w.checkme(10);
	}

	TEST(Test_Seaweeds_WindowlocalLCS_Allmismatch) {
		Seaweeds::string t(109);
		Seaweeds::string p(10);

		for (int i = 0; i < t.size(); ++i)	{
			t[i] = 0;
		}
		for (int i = 0; i < p.size(); ++i)	{
			p[i] = 1;
		}

		wr w;
		Seaweeds sw(10, p, 1);
		sw.count(t, &w, 0, 5);		
		w.checkme(0);
	}

	SUITE(Lengthy) {
		TEST(Test_Random_Seaweed_Windowlocal_LCS)
		{
			init_xasmlib();
			for (int l = 10; l < 10+Seaweeds::max_windowlength; l += 17) {
				int grid = 1;
				for (int g = 0; g < 4; ++g) {
					for (int w = max(grid,2); w < min((int)Seaweeds::max_windowlength, l); w+= grid) { 
						for (int p = w; p <= l; p+=grid) {
							if ((p + w)/grid <= 2*Seaweeds::max_windowlength) {
								test_wllcs(l, p, w, grid);
							}
								
						}
					}
					grid*= 2;
				}
			}
		}
	}
};
