/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef POSTPROCESSWINDOWS_H_
#define POSTPROCESSWINDOWS_H_

#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <string.h>

#include <boost/regex.hpp>

#include "bspcpp/tools/utilities.h"
#include "bspcpp/ParameterFile.h"
#include "bspcpp/CommandLine.h"

namespace postprocess {

using namespace std;

typedef struct _window {
	size_t x;
	size_t y;
	double score;
	string val;
} window_location;

bool operator==(_window const & w1, _window const & w2) {
	return w1.x == w2.x && w1.y == w2.y;
}

struct score_lt {
	double threshold;
	score_lt(double _t) : threshold (_t) {}

	bool operator() (pair<string, _window> const & w1) {
		return w1.second.score < threshold;
	}
};

struct win_lt {
	bool operator()(_window const & w1, _window const & w2) {
		return strcmp(w1.val.c_str(), w2.val.c_str()) < 0;
	}
};

inline void postprocess(int argc, const char ** argv) {
	using namespace std;
	using namespace utilities;

	int files_read = 0;
	int max_windows = 10000;
	int th = 0;

	CommandLine cmd(argc, argv);
	cmd.getValue("m", max_windows);
	
	map<string, _window> windows;
	double threshold = 0;
	cmd.getValue("t", th);

	threshold = th;
	int win_x_min= 0;
	int win_x_max= 0;
	int win_y_min= 0;
	int win_y_max= 0;

	cout << "Starting threshold: " << threshold << endl;

	for(int file = 1; file < argc; ++file) {
		ifstream in(argv[file]);
		if(argv[file][0] == '-') {
			break;
		}
		if(in.bad() || !in.is_open()) {
			continue;
		} else {
			cout << "Reading " << argv[file] << endl;
		}

		string line = getline(in);
		if(threshold <= 0) {
			threshold = atoi(line.c_str());
		}
		int lines  = 0;
		while(!in.eof()) {
			line = getline(in);
			if(line.size() == 0) {
				continue;
			}
			boost::cmatch match;
			boost::regex ex("([0-9]+)[ \\t]+([0-9]+)[ \\t]+(.+)");
			if(!boost::regex_match(line.c_str(), match, ex)) {
				cerr << "[W] cannot interpret line: " << line << endl;
				continue;
			}
			string s1 (match[1].str());
			string s2 (match[2].str());
			string s3 (match[3].str());

			_window wl;
			wl.x = atoi(s1.c_str());
			wl.y = atoi(s2.c_str());
			wl.score = atof(s3.c_str());
			char c[512];
			sprintf(c, "%g,%i,%i", wl.score, (int)wl.x, (int)wl.y);
			wl.val = c;

			++lines;

			if(wl.score >= threshold) {
				windows[wl.val] = wl;
			} 

			while(windows.size() > (unsigned) max_windows) {
				threshold+= 1;
				map<string, _window>::iterator it = find_if(windows.begin(), windows.end(), score_lt(threshold));
				while(it != windows.end()) {
					windows.erase(it);
					it = find_if(windows.begin(), windows.end(), score_lt(threshold));
				}
				cout << "Threshold raised: " << threshold << " n = " << windows.size() << endl;
			}
		}
		in.close();
		cout << "... read " << lines << " lines " << endl;
	}

	ofstream o;

	string n;
	if(cmd.hasValue("o")) {
		cmd.getValue("o", n);
	} else {
		n = argv[1];
	}
	cout << "Writing to file " << n << endl;
	o.open(n.c_str(), ios::out);

	std::vector<_window> windows_vec(windows.size());
	int j = 0;
	for(std::map<string, _window>::iterator it = windows.begin();
		it != windows.end();
		++it) {
		windows_vec[j++] = it->second;
	}

	sort(windows_vec.begin(), windows_vec.end(), win_lt());

	o << threshold << endl;
	for (std::vector<_window>::iterator it = windows_vec.begin(); it != windows_vec.end(); ++it) {
		win_x_min = min(win_x_min, (int)it->x);
		win_y_min = min(win_y_min, (int)it->y);
		win_x_max = max(win_x_max, (int)it->x);
		win_y_max = max(win_y_max, (int)it->y);
		o << it->x << "\t" << it->y << "\t" << it->score << endl;
	}
	o << flush;
	o.close();

	cout << "Number of windows after postprocessing: " << windows.size() << endl;
}

};

#endif /* POSTPROCESSWINDOWS_H_ */
