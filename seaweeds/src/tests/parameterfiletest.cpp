/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include <iostream>

#include "bspcpp/ParameterFile.h"

using namespace std;
using namespace utilities;

int main(int argc, char ** argv) {
	ParameterFile f;

	cout << "Initial:" << endl;
	f.set("test1", 0);
	f.set("test2", 1.2);
	f.set("test3", string("abc"));
	f.set("test4", 1);
	f.set("test5", 2.4);
	f.set("test6", string("def"));
	f.dump(std::cout);
	cout << endl;

	f.write("binary_test.dat", ParameterFile::BINARY);
	f.write("ascii_test.dat", ParameterFile::ASCII);

	cout << "From binary:" << endl;
	f.clear();
	f.read("binary_test.dat", ParameterFile::BINARY);
	f.dump(std::cout);
	cout << endl;

	cout << "From ascii:" << endl;
	f.clear();
	f.read("ascii_test.dat", ParameterFile::ASCII);
	f.dump(std::cout);
	cout << endl;

	cout << "test4 set to 2" << endl;
	f.set("test4", 2);
	f.writeupdate("ascii_test.dat", ParameterFile::ASCII);

	cout << "reading ascii update" << endl;
	f.clear();
	f.read("ascii_test.dat", ParameterFile::ASCII);
	f.dump(std::cout);
	cout << endl;

	cout << "test5 set to 4.5" << endl;
	f.set("test5", 4.5);
	f.writeupdate("binary_test.dat", ParameterFile::BINARY);

	cout << "reading binary update" << endl;
	f.clear();
	f.read("binary_test.dat", ParameterFile::BINARY);
	f.dump(std::cout);
	cout << endl;

	return 0;
}
