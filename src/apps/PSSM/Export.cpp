/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "PSSM_Tool.h"

#include <fstream>

void PSSM_ExportApp::run(boost::program_options::variables_map & vm) {
	using namespace std;

	std::string filename = vm["output-file"].as<std::string>();
	cerr << "Writing to " << filename << endl;

	ofstream out(filename.c_str());

	for (int j = 0; j < g_pssms.size(); ++j) {
		std::ostringstream oss;
		oss << g_pssms[j];
		std::string st(oss.str());
		std::replace(st.begin(), st.end(), '\n', ' ');
		std::replace(st.begin(), st.end(), '\r', ' ');
		out << st << "\n";
	}
}
