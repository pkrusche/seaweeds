/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "PSSM_Tool.h"

#include <boost/filesystem.hpp>

#include "datamodel/SerializePrimitives.h"

/************************************************************************/
/* Profile PSSM matrices                                                */
/************************************************************************/

void PSSM_ProfileApp::get_profile (
	pssm & p, 
	std::string const & scoretype, 
	pssmhistogram & hist,
	std::string const & ppath) {
	using namespace std;
	using namespace boost::filesystem;
		
	path profilepath (ppath);
	{
		ostringstream namestr;
		namestr << "profile_" << p.get_accession() << "_"  << p.get_name() << "_" << g_background_id
			<< "_" << scoretype << ".json";
		std::string fname = namestr.str();
		profilepath/= path(fname);
	}
		
	PSSM_Profile prof;
	if (exists (profilepath)) {
		std::ifstream p_in ( profilepath.string().c_str() );
		try {
			if (p_in.good()) {
				datamodel::sread< datamodel::JSONSerializable < PSSM_Profile > > rd; 
				rd ( p_in, prof );
			}				
		} catch (std::exception e) {
			// ignore, recompute.
		}
	}

	std::map<std::string, pssmhistogram>::iterator it = prof.histograms.find (scoretype);
	if (it == prof.histograms.end()) {
		pssmscore score = pssmscorefactory::create(
			p, 
			pssmscorefactory::s(scoretype.c_str())
		);

		hist.make_histogram ( score.get(), g_background.get() );
		prof.histograms[scoretype] = hist;
		
		std::ofstream p_out ( profilepath.string().c_str() );
		datamodel::swrite< datamodel::JSONSerializable < PSSM_Profile > > wr; 
		wr ( p_out, prof );		
	} else {
		hist = it->second;
	}
}

void PSSM_ProfileApp::run (boost::program_options::variables_map & vm) {
	using namespace std;
	double pval = vm["pval"].as<double>();
	string scoretype = vm["scoretype"].as<string>();
		
	{	// check profiles directory
		using namespace boost::filesystem;
		path profilespath (vm["profiles"].as<string>());

		if (!exists (profilespath) || !is_directory(profilespath) ){
			throw std::runtime_error ("Profile directory could not be found.");
		}			
	}

	start_progress ("Making PSSM profiles...", (int)g_pssms.size());
	for ( size_t j = 0; j < g_pssms.size(); ++j ) {
		pssmhistogram hist;
		get_profile (g_pssms[j], scoretype, hist, vm["profiles"].as<string>());
		add_progress(1);
	}
	end_progress();
}

