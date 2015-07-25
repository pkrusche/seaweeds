/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "PSSM_Tool.h"

void PSSM_InfoApp::run(boost::program_options::variables_map & vm) {
	using namespace std;
	cout << "#NAME\tLENGTH\tENTROPY" << endl;
	for (int j = 0; j < g_pssms.size(); ++j) {
		pssm & p (g_pssms[j]);
		cout << p.get_name() << "\t" << p.get_length() << "\t"<< p.entropy() << endl;
	}
}
