/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <string>
#include <cstdlib>


#include "UnitTest++.h"

#define LCS_VERIFY

#include "lcs/Llcs.h"
#include "lcs/LlcsCIPR.h"

TEST(Test_LCS_Short) {
	using namespace std;
	
	lcs::Llcs<> lcs;
	lcs::LlcsCIPR<8> blcs;

	string s1;
	string s2;

	for(int j = 0; j < 1000; ++j) {
		s1 = "";
		s2 = "";

		int lcslen = 0;
		for (int k = 0; k < 100; ++k) {
			char c1 = (rand()*2/RAND_MAX) > 1 ? 'A' : 'X';
			char c2 = (rand()*2/RAND_MAX) > 1 ? 'A' : 'Y';
			s1 += c1;
			s2 += c2;
			if (c1 == c2) {
				++lcslen;
			}
		}
		
		CHECK_EQUAL(lcslen, lcs(s1, s2));

		lcs::LlcsCIPR<8>::string bs1(s1);
		lcs::LlcsCIPR<8>::string bs2(s2);

		CHECK_EQUAL(lcslen, blcs(bs1, bs2));		
	}
}
