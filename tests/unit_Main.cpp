/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"
#include <cstring>

#include "UnitTest++.h"
#include "TestReporterStdout.h"

using namespace UnitTest;

struct specific_test
{
	specific_test(const char * _name) : name(_name) {}

	bool operator() (const Test * const test) const {
		if (test == NULL || test->m_details.testName == NULL || name == NULL)
		{
			return false;
		} else if(strlen(name) > 1 && name[strlen(name)-1] == '*') {			
			ASSERT(strlen(name) < 2048);
			char cn[2048];
			memcpy(cn, name, strlen(name));
			cn[strlen(name)-1] = 0;
			const char * m = strstr(test->m_details.testName, cn);
			return m != NULL;
		} else {
			return strcmp(test->m_details.testName, name) == 0;			
		}
	}

	const char * name;
};

int RunStandardTests() {
	TestReporterStdout reporter;
	TestRunner runner(reporter);
	return runner.RunTestsIf(Test::GetTestList(), "DefaultSuite", True(), 0);
}

int RunLengthyTests() {
	TestReporterStdout reporter;
	TestRunner runner(reporter);
	return runner.RunTestsIf(Test::GetTestList(), "Lengthy" , True(), 0);
}

int RunSpecificTest(const char * name) {
	TestReporterStdout reporter;
	TestRunner runner(reporter);
	return 
		runner.RunTestsIf(Test::GetTestList(), "Lengthy" , specific_test(name), 0)
	||  runner.RunTestsIf(Test::GetTestList(), "DefaultSuite" , specific_test(name), 0);
}

int main(int argc, char const * argv[])
{
	if (argc > 1) {
		if ( strcmp (argv[1], "lengthy") == 0 ) {
			return 
				RunStandardTests() + RunLengthyTests();
		}
		if ( strcmp (argv[1], "only") == 0 ) {
			int r = 0;
			for (int i = 2; i <= argc; ++i)	{
				if(argv[i]!= NULL)
					r += RunSpecificTest(argv[i]);
			}
			return r;
		}
	} else {
    	return RunStandardTests();		
	}
}
