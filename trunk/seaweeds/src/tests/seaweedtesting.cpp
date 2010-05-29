/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#define _VERBOSETEST
#define _DEBUG_SEAWEEDS

#include <iostream>
#include <fstream>
#include <loki/Typelist.h>

#include "autoconfig.h"

#include "bspcpp/tools/utilities.h"
#include "seaweeds/ScoreMatrix.h"
#include "lcs/Llcs.h"

using namespace std;
using namespace seaweeds;

// Reference implementation
#define SCOREMATRIX_E	ScoreMatrix<ExplicitStorage<lcs::Llcs< > > >
#define SCOREMATRIX_I	ScoreMatrix<ImplicitStorage < > >

template<class SCOREMATRIX>
void test() {
	SCOREMATRIX m(11,12);
	SCOREMATRIX m2(11,12);

	m.identity();

	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

	m.semilocallcs("AACBABC", "CABABC");

	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

	m.incremental_semilocallcs("BACA", SCOREMATRIX::APPEND_TO_X);

	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

	m.incremental_semilocallcs("ABBBAB", SCOREMATRIX::APPEND_TO_Y);

	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

	m.reverse_xy();
	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

#ifdef TEST_REFERENCE
	m.semilocallcs("BABCAACBABCBACA", "CCBACABABCABBBAB"); 
#else
	m.incremental_semilocallcs("ABCC", SCOREMATRIX::APPEND_TO_Y);
	m.incremental_semilocallcs("CBAB", SCOREMATRIX::APPEND_TO_X);
	m.reverse_xy();
#endif
	cout << "x = " << m.get_x() << endl;
	cout << "y = " << m.get_y() << endl;
	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

	cout << endl << endl;
	cout << "ABAD vs AB" << endl;
	m.semilocallcs("ABAD", "AB");

	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

	cout << "ABAD vs BD" << endl;
	m.semilocallcs("ABAD", "BD");

	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);


	cout << "ABAD vs ABBD" << endl;
	m.semilocallcs("ABAD", "ABBD");

	SCOREMATRIX::dumpdensity(cout, m);
	SCOREMATRIX::dumpdistribution(cout, m);
	SCOREMATRIX::dumpscore(cout, m);

}

int main(int argc, char *argv[]) {
	using namespace std;
	utilities::init_xasmlib();

#ifdef TEST_REFERENCE
	cout << "Seaweed test, using reference (explicit) implementation." << endl;
	test<SCOREMATRIX_E>();
#else
	cout << "Seaweed test, using implicit implementation." << endl;
	test<SCOREMATRIX_I>();
#endif
	return EXIT_SUCCESS;
}
