/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cfloat>

#include "UnitTest++.h"

#include "apps/ParameterFile.h"

using namespace UnitTest;
using namespace std;
using namespace utilities;

namespace {

TEST(Test_Parameter_RW) {
	ParameterFile pf;


	// int
	char n[256];
	int  vals[100];
	
	for (int i = 0; i < 100; ++i) {
		vals[i] = rand();
		sprintf(n, "int_%i", i);
		pf.set(n, vals[i]);
	}

	for (int i = 0; i < 100; ++i) {
		int r = -1;
		sprintf(n, "int_%i", i);
		pf.get(n, r, -2);

		CHECK_EQUAL(r, vals[i]);
	}

	// double
	double  dvals[100];
	
	for (int i = 0; i < 100; ++i) {
		dvals[i] = 1.0 * rand() / RAND_MAX;
		sprintf(n, "dbl_%i", i);
		pf.set(n, dvals[i]);
	}

	for (int i = 0; i < 100; ++i) {
		double r = -1;
		sprintf(n, "dbl_%i", i);
		pf.get(n, r, -2);

		CHECK_CLOSE(r, dvals[i], DBL_EPSILON);
	}

	// string
	for (int i = 0; i < 100; ++i) {
		sprintf(n, "str_%i", i);
		pf.set(n, n);
	}

	for (int i = 0; i < 100; ++i) {
		std::string rr;
		sprintf(n, "str_%i", i);
		pf.get(n, rr, "xxx");

		CHECK(strcmp(rr.c_str(), n) == 0);
	}

	char buffer [2048];
#ifdef _WIN32
	strcpy(buffer, "__PARAMETERFILETEST.txt");
#else
	tmpnam (buffer);
#endif

	pf.write(buffer);

	ParameterFile f2;
	f2.read(buffer);

	for (int i = 0; i < 100; ++i) {
		int r = -1;
		sprintf(n, "int_%i", i);
		f2.get(n, r, -2);

		CHECK_EQUAL(r, vals[i]);
	}
	for (int i = 0; i < 100; ++i) {
		double r = -1;
		sprintf(n, "dbl_%i", i);
		f2.get(n, r, -2);

		CHECK_CLOSE(r, dvals[i], 0.001);
	}
	for (int i = 0; i < 100; ++i) {
		std::string rr;
		sprintf(n, "str_%i", i);
		f2.get(n, rr, "xxx");

		CHECK(strcmp(rr.c_str(), n) == 0);
	}
	remove(buffer);
}

};
