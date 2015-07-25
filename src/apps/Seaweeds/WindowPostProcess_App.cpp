/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "WindowPostProcess_App.h"

#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <string.h>


#include <boost/regex.hpp>

using namespace std;

/**
 * Window sorting / duplicate removal helpers
 */

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

void WindowPostProcess_App::run( boost::program_options::variables_map & vm ) {
	using namespace std;

	int files_read = 0;
	int max_windows = 10000;
	int th = 0;
	int processors = 1;
	string output_file = "result.txt";

	max_windows = vm["max-windows"].as<int>();
	th          = vm["cutoff"].as<int>();
	output_file = vm["output-file"].as<string>();
	processors  = vm["processors"].as<int>();

	map<string, _window> windows;
	double threshold = th;

	int win_x_min= 0;
	int win_x_max= 0;
	int win_y_min= 0;
	int win_y_max= 0;

	cout << "Starting threshold: " << threshold << endl;

	for (int p = 0; p < processors; ++p) {	
		string file;
		{
			stringstream oss;
			oss << output_file << "_" << p;
			file = oss.str();
		}
		ifstream in(file.c_str());

		if(in.bad() || !in.is_open()) {
			continue;
		} else {
			cout << "Reading " << file << endl;
		}

		string line ("");
		getline(in, line, '\n');
		if(threshold <= 0) {
			threshold = atoi(line.c_str());
		}
		int lines  = 0;
		while(in.good()) {
			line = "";
			if(!getline(in, line, '\n')) {
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
	cout << "Writing to file " << output_file << endl;
	o.open(output_file.c_str(), ios::out);

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

