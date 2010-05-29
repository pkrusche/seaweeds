/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __COMMANDLINE_H__
#define __COMMANDLINE_H__

#include <map>
#include <string>
#include <iostream>

namespace utilities {

class CommandLine {
public:
	CommandLine();
	CommandLine(int argc, const char * argv[]);

	bool hasValue(std::string name);

	bool getValue(std::string name, int & value);
	bool getValue(std::string name, float & value);
	bool getValue(std::string name, std::string & value);
	bool getNonemptyValue(std::string name, std::string & value);

	std::string getExecutable();
	std::string getMyPath();
	std::string getCwd();

	friend std::ostream & operator <<(std::ostream & o, CommandLine const & c);
private:
	std::map<std::string, std::string> values;
	std::string executable;
	std::string cwd;
	std::string mypath;
};

};

#endif
