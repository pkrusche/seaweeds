/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string>

#include "bspcpp/CommandLine.h"

#include "autoconfig.h"

namespace utilities {

extern std::string trimstring(const std::string & g, char c);

CommandLine::CommandLine() {

}

CommandLine::CommandLine(int argc, const char * argv[]) {
	executable = argv[0];
	mypath = argv[0];
	char cwdbuffer[2048];

	char * tmp = getcwd(cwdbuffer, 2048);
	cwd = cwdbuffer;

	size_t lp;
	lp = executable.find_last_of('/');
	if(lp == std::string::npos) {
		lp = executable.find_last_of('\\');
	}
	if(lp != std::string::npos) {
		executable = executable.substr(lp + 1);
		mypath = mypath.substr(0, lp);
	} else {
		mypath = cwd;
	}

	enum {
		START,
		PARAMETER,
	} parser_state = START;

	std::string parametername = "";

	for (int k = 1; k < argc; ++k) {
		if(argv[k][0] == '-' || argv[k][0] == '/'  || argv[k][0] == '+') {
			parser_state = PARAMETER;
			parametername = argv[k];
			parametername = parametername.substr(1);
			values[parametername] = "";
		} else
		if(parser_state == PARAMETER) {
			values[parametername]+= " ";
			values[parametername]+= argv[k];
		}
	}
	for (std::map<std::string, std::string>::iterator it = values.begin();
		it != values.end();
		++it) {
		it->second = trimstring(it->second, ' ');
	}
}

bool CommandLine::hasValue(std::string name) {
	std::map<std::string, std::string>::iterator it = values.find(name);
	if(it != values.end()) {
		return true;
	} else {
		return false;
	}
}

bool CommandLine::getValue(std::string name, int & value) {
	std::map<std::string, std::string>::iterator it = values.find(name);
	if(it != values.end()) {
		value = atoi(it->second.c_str());
	} else {
		return false;
	}
	return true;
}

bool CommandLine::getValue(std::string name, float & value) {
	std::map<std::string, std::string>::iterator it = values.find(name);
	if(it != values.end()) {
		value = (float)atof(it->second.c_str());
	} else {
		return false;
	}
	return true;
}

bool CommandLine::getValue(std::string name, std::string & value) {
	std::map<std::string, std::string>::iterator it = values.find(name);
	if(it != values.end()) {
		value = it->second;
	} else {
		return false;
	}
	return true;
}

bool CommandLine::getNonemptyValue(std::string name, std::string & value) {
	std::map<std::string, std::string>::iterator it = values.find(name);
	if(it != values.end() && it->second != "") {
		value = it->second;
	} else {
		return false;
	}
	return true;
}

std::string CommandLine::getExecutable() {
	return executable;
}

std::string CommandLine::getMyPath() {
	return mypath;
}

std::string CommandLine::getCwd() {
	return cwd;
}

std::ostream & operator <<(std::ostream & o, CommandLine const & c) {
	o << "exec: " << c.executable <<
		 " path: " << c.mypath <<
		 " cwd : " << c.cwd <<
		 std::endl;
	for (std::map<std::string, std::string>::const_iterator it = c.values.begin();
		 it != c.values.end();
		 ++it) {
		o << it->first << " = " << it->second << std::endl;
	}

	return o;
}

};
