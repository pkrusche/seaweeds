/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __PSSM_Tool_H__
#define __PSSM_Tool_H__

#include <iostream>
#include <fstream>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <bsp_cpp/bsp_cpp.h>

#include "xasmlib/IntegerVector.h"

#include "pssms/Pssm.h"
#include "sequencemodel/SequenceModel.h"
#include "sequencemodel/MarkovModel.h"

#include "../App.h"

/************************************************************************/
/* Typedefs for this code                                               */
/************************************************************************/

/** We store DNA in 8-bit strings (consumes memory, better performance than using 2-3 bits) */
typedef utilities::IntegerVector<8> dnastring;

/** PSSM for DNA with default alphabet size (4) */
typedef pssms::PSSM<> pssm;

/** PSSM Scoring histograms for our string type */
typedef pssms::PSSM_Histogram< dnastring > pssmhistogram;

/** This factory produces our score functions */
typedef pssms::PSSM_Score_Factory<dnastring, 4> pssmscorefactory;

/** PSSM Score function type */
typedef pssmscorefactory::product_t pssmscore;

/** Sequence models are stored in a shared pointer */
typedef boost::shared_ptr < sequencemodel::SequenceModel< dnastring > > pssmsequencemodel;

/** we need this as a shortcut for allocation */
typedef sequencemodel::MarkovModel<2,8> pssmmarkovmodel;


/************************************************************************/
/* APP globals                                                          */
/************************************************************************/

class PSSM_Profile : public datamodel::Serializable {
public:
	std::map<std::string, pssmhistogram> histograms;

	PSSM_Profile const& operator=(PSSM_Profile const& rhs) {
		if (this == &rhs) {
			return *this;
		}
		histograms = rhs.histograms;
		this->datamodel::Serializable::operator=( rhs );
		return *this;
	}

	JSONIZE(PSSM_Profile, 0, 
		S_STORE(histograms, datamodel::JSONMap < datamodel::JSONSerializable<pssmhistogram> > )
		);
};

/** The PSSM Tool works on a set of PSSMs, which are stored here */
extern std::vector < pssm > g_pssms;

/** The PSSM Tool automatically reads or creates a background model */
extern pssmsequencemodel g_background;

/** Here is an id for the background model being used */
extern std::string g_background_id;

/************************************************************************/
/* PSSM Tool Apps                                                       */
/************************************************************************/

/**
 * Compare PSSM Scoring functions.
 */
class PSSM_CompareScoresApp : public App  {
public:
	void add_opts( boost::program_options::options_description & all_opts ) {
		using namespace std;
		namespace po = boost::program_options;
		po::options_description opts("PSSM Score comparison options");
		opts.add_options()
			("output,o", po::value<string>()->default_value("output.txt"), 
			"Score comparison output will be written to this file (default: output.txt)");
		all_opts.add(opts);
	}

	void run( boost::program_options::variables_map & vm );
};

/**
 * App class for PSSM distances
 */
 class PSSM_DistancesApp : public App {
 public:

	 /** add options */
	 void add_opts( boost::program_options::options_description & all_opts ) {
		 namespace po = boost::program_options;
		 po::options_description opts ("PSSM Distances Options");

		 opts.add_options()
			 ("distancetype,t", po::value<int>()->default_value(0), 
			 "Specify the distance function to use -- 0 (Hellinger) or 1 (Kullback-Leibler)")
			 ("normalize,N",	"Normalize distance matrix before output.");
		all_opts.add( opts );
	 }

	 /** run the app and output results on one node */
	 void run (boost::program_options::variables_map & vm);
 };

 /**
  * App to create profile database folder
  */
 class PSSM_ProfileApp : public App  {
 public:

	 /**
	  * Get a profile for a pssm and score type.
	  * 
	  * @param p the pssm
	  * @param scoretype the Score type to give to the factory
	  * @param hist the histogram to write to
	  * @param ppath the path of the profle directory
	  */
	 static void get_profile (pssm & p, std::string const & scoretype, pssmhistogram & hist, std::string const & ppath );

	 /** add options */
	 void add_opts( boost::program_options::options_description & all_opts ) {
		 using namespace std;
		 namespace po = boost::program_options;
		 po::options_description opts("PSSM Profiling Options");

		 opts.add_options()
			 ("pval,p", po::value<double>()->default_value(0.01),
			 "Best p-value threshold for profiling.")
			 ("scoretype", po::value<std::string>()->default_value("mult"),
			 "Which score to use (you can specify the pseudocount type as well: mult none, mult linear, mult sqrt, or mult bifa.")
			 ("profiles", po::value<std::string>()->default_value("profiles"),
			 "Directory name of the profiles database.")
			 ;

		 all_opts.add(opts);
	 }

	 void run(boost::program_options::variables_map & vm);
 };

 /**
  * Parallel scoring of PSSMs on an input sequence file.
  */

 class PSSM_ScoreApp : public App  {
 public:
	 void add_opts( boost::program_options::options_description & all_opts ) {
		 using namespace std;
		 namespace po = boost::program_options;
		 po::options_description opts("PSSM Scoring Options");

		 opts.add_options()
			 ("sequence", po::value<std::string>()->default_value(""),
			 "Name of the input sequence file.")
			 ("fragmentsize", po::value<size_t>()->default_value(4096),
			 "Size of the input chunks to read (increase this for larger input sequences).")
			 ("binomial_p", po::value<double>()->default_value(0.05),
			 "Binomial test p-value limit (-1 to disable test).")
			 ("verbosity", po::value<int>()->default_value(0),
			 "Level of verbosity.")
			;

		 all_opts.add(opts);
	 }
	 
	 void run(boost::program_options::variables_map & vm);
 };

 /**
  * Output data about score matrices in tabular format.
  */

 class PSSM_InfoApp : public App  {
 public:
	 void add_opts( boost::program_options::options_description & all_opts ) {
		// uses -f and -a
	 }
	 
	 void run(boost::program_options::variables_map & vm);
 };

 /**
  * Output data about score matrices in tabular format.
  */

 class PSSM_ExportApp : public App  {
 public:
	void add_opts( boost::program_options::options_description & all_opts ) {
		// uses -f and -a
		using namespace std;
		namespace po = boost::program_options;
		po::options_description opts("PSSM Export Options");

		opts.add_options()
			 ("output-file", po::value<std::string>()->default_value("output.mjson"),
			 	"Name of mjson output file to write.")
			;

		all_opts.add(opts);
	}
	 
	void run(boost::program_options::variables_map & vm);
 };


#endif // __PSSM_Tool_H__
