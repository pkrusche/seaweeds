/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include "PSSM_Tool.h"

/************************************************************************/
/* Compare PSSM Scoring functions. Sequential code.                     */
/************************************************************************/

/** run computation */
void PSSM_CompareScoresApp::run( boost::program_options::variables_map & vm ) {
	// sequential app
	if (bsp_pid() != 0) {
		return;
	}

	using namespace std;
	using namespace utilities;
	using namespace sequencemodel;
	using namespace pssms;

	double pval = vm["pval"].as<double>();

	std::string output = vm["output"].as<string>();
	std::string profiles = vm["profiles"].as<string>();

	std::cerr << "Writing to " << output << endl;

	list<string> scores;
	scores.push_back("mult none");
	scores.push_back("mult linear");
	scores.push_back("mult sqrt");
	// scores.push_back("mult bifa");

	ofstream out(output.c_str());

	out << "PSSM Name" << "\t" 
		<< "length" << "\t" 
		<< "entropy";

	using namespace boost::accumulators;
	typedef accumulator_set<double, stats<tag::mean, tag::moment<2>, tag::error_of<tag::mean> > > statacc;
	map<std::string, statacc> accs;
	for (list<string>::iterator it = scores.begin(); it != scores.end(); ++it ) {
		out << "\t" << *it;
		accs[*it] = statacc();
	}
	out << endl;

	start_progress("Profiling all matrices...", (int)g_pssms.size());
	for ( size_t j = 0; j < g_pssms.size(); ++j ){
		out << g_pssms[j].get_name() << "\t" << g_pssms[j].get_length () 
			<< "\t" << g_pssms[j].entropy();

		for (list<string>::iterator it = scores.begin(); it != scores.end(); ++it ) {
			pssmhistogram hist;
			PSSM_ProfileApp::get_profile (g_pssms[j], *it, hist, profiles);
			
			double min_score = hist.right_tail_min_score(pval);
			out << "\t" << min_score;
			accs[*it](min_score);
		}
		out << endl;
		
		add_progress(1);
	}
	end_progress();

	for (list<string>::iterator it = scores.begin(); it != scores.end(); ++it ) {
		std::cerr << *it << ": mean=" << mean(accs[*it]) << " err=" << error_of<tag::mean>(accs[*it]) << endl;
	}
}
