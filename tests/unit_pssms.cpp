/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "UnitTest++.h"

#include "datamodel/TextIO.h"
#include "datamodel/SequenceTranslation.h"
#include "pssms/Pssm.h"
#include "sequencemodel/MarkovModel.h"

using namespace UnitTest;

using namespace std;
using namespace datamodel;
using namespace sequencemodel;
using namespace pssms;
using namespace utilities;

namespace {

	typedef IntegerVector<8> stored_sequence;

	template <class scorefun, class _PSSM> 
	void test_score (_PSSM & testpssm, scorefun & ps, int len, 
		int trials, const char * fn = "profile.dat") {
		MarkovModel<2> mm;

		int eff_len = len - testpssm.get_length();

		double * profile_avg= new double [eff_len];
		memset (profile_avg, 0, sizeof (double) * eff_len);

		for (int j = 0; j < trials; ++j) {
			stored_sequence v;
			v = mm.generate_sequence(len);
			testpssm.random_sequence (v, len/2 - 6);
			for (int k = 0; k < eff_len; ++k) {
				profile_avg[k]+= ps.score(v, k);
			}

		}
		for (int k = 0; k < eff_len; ++k) {
			profile_avg[k]/= trials;
		}

		ofstream profile (fn);

		for (int k = 0; k < eff_len; ++k) {
			profile << profile_avg[k] << endl; // "\t" << profile_dev[k] << endl;
			// std::cerr << fn << "\t" << k << "\t" << len << std::endl;
			// values outside the match region should be quite low
			if (k <= ((len-6)/2 - 6) || k > ((len-6)/2 + 3)) {
				CHECK ( profile_avg [k] < 0.6 );
			} else if(k == (len-6)/2-3) {  // there should be a peak in the middle
				CHECK ( profile_avg [k] > 0.8 );
			}

		}
		profile << endl;

		delete [] profile_avg;
		
	}

	const char* test_pssm_json ();
	const char* test_pssm_jaspar();
		
	TEST(Test_PSSM_Multiplicative_Scoring) {

		PSSM <4> testpssm; 
		istringstream str (test_pssm_json());
		str >> testpssm;

		PSSM_Multiplicative_Score<stored_sequence, 4> ms_no_zc (testpssm, PSSM<4>::PSEUDOCOUNT_NONE);
		test_score<PSSM_Multiplicative_Score<stored_sequence, 4>, PSSM<4> >(testpssm, ms_no_zc, 100, 500, "examples/pssms/profile_multiplicative_no_zc.dat");
		PSSM_Multiplicative_Score<stored_sequence, 4> ms_boris_zc (testpssm, PSSM<4>::PSEUDOCOUNT_SQRT);
		test_score<PSSM_Multiplicative_Score<stored_sequence, 4>, PSSM<4> >(testpssm, ms_boris_zc, 100, 500, "examples/pssms/profile_multiplicative_boris_zc.dat");
		PSSM_Multiplicative_Score<stored_sequence, 4> ms_sascha_zc (testpssm, PSSM<4>::PSEUDOCOUNT_LINEAR);
		test_score<PSSM_Multiplicative_Score<stored_sequence, 4>, PSSM<4> >(testpssm, ms_sascha_zc, 100, 500, "examples/pssms/profile_multiplicative_sascha_zc.dat");
	}

	SUITE(Lengthy) {

	TEST(Test_Profile_Serialization) {
		PSSM <4> testpssm; 
		istringstream str (test_pssm_json());
		str >> testpssm;

		MarkovModel<2, 8> mm;

		PSSM_Multiplicative_Score<IntegerVector<8>, 4> bs (testpssm);

		PSSM_Histogram< IntegerVector<8> > hist1;
		hist1.make_histogram(&bs, &mm );
		stringstream oss;
		oss << hist1;
		PSSM_Histogram< IntegerVector<8> > hist2;
		oss >> hist2;

		PSSM_Histogram< IntegerVector<8> > hist3 = hist1;

		for (double k = 0.85; k <= 1; k += 0.01) {
			CHECK_CLOSE(hist1.p(k-0.1, k), hist2.p(k-0.1, k), DBL_EPSILON);
			CHECK_CLOSE(hist1.right_tail(k), hist2.right_tail(k), DBL_EPSILON);
			CHECK_CLOSE(hist1.right_tail_min_score(k), hist2.right_tail_min_score(k), DBL_EPSILON);
			CHECK_CLOSE(hist1.right_tail_min_score(k), hist3.right_tail_min_score(k), DBL_EPSILON);
			CHECK_CLOSE(hist1.p(k-0.1, k), hist3.p(k-0.1, k), DBL_EPSILON);
			CHECK_CLOSE(hist1.right_tail(k), hist3.right_tail(k), DBL_EPSILON);
			CHECK(hist2.right_tail(k) < 1.0);
		}
	}

	TEST(Test_Scoring_Profiles) {
		PSSM <4> testpssm; 
		istringstream str (test_pssm_json());
		str >> testpssm;

		MarkovModel<2,8> mm;

		PSSM_Multiplicative_Score<IntegerVector<8>, 4> ms_no_zc (testpssm, PSSM<4>::PSEUDOCOUNT_NONE);
		PSSM_Multiplicative_Score<IntegerVector<8>, 4> ms_boris_zc (testpssm, PSSM<4>::PSEUDOCOUNT_SQRT);
		PSSM_Multiplicative_Score<IntegerVector<8>, 4> ms_sascha_zc (testpssm, PSSM<4>::PSEUDOCOUNT_LINEAR);

		PSSM_Histogram< IntegerVector<8> > hist1, hist2a, hist2b, hist2c, hist2d;
		hist2a.make_histogram(&ms_no_zc, &mm);
		hist2b.make_histogram(&ms_boris_zc, &mm);
		hist2c.make_histogram(&ms_sascha_zc, &mm);

		ofstream pr ("examples/pssms/quantiles.dat");
		pr  << "k" << "\t" 
			<< "min_mult_no_zc( p >= k )" << "\t" 
			<< "mult_no_zc_density_score(p =~ k)" << "\t" 
			<< "mult_no_zc_cdf_score(p >= k)" << "\t" 
			<< "min_mult_boris_zc( p >= k )" << "\t" 
			<< "mult_boris_zc_density_score(p =~ k)" << "\t" 
			<< "mult_boris_zc_cdf_score(p >= k)" << "\t" 
			<< "min_mult_sascha_zc( p >= k )" << "\t" 
			<< "mult_sascha_zc_density_score(p =~ k)" << "\t" 
			<< "mult_sascha_zc_cdf_score(p >= k)" << "\t" 
			<< endl;
		for (double k = 0.005; k <= 1; k += 0.005) {
//			cerr << k << endl;
			pr << k << "\t" << hist2a.right_tail_min_score(k) << "\t" << hist2a.p(k-0.001, k) << "\t" << hist2a.right_tail(k)
					<< "\t" << hist2b.right_tail_min_score(k) << "\t" << hist2b.p(k-0.001, k) << "\t" << hist2b.right_tail(k)
					<< "\t" << hist2c.right_tail_min_score(k) << "\t" << hist2c.p(k-0.001, k) << "\t" << hist2c.right_tail(k)
				    << endl;
		}

	}
	
	}
	
	TEST(Test_JASPAR_IO) {
		using namespace std;
		PSSM <4> testpssm; 
		istringstream str (test_pssm_jaspar());
		testpssm.load_jaspar_pssm(str);
		ofstream fout("examples/pssms/jaspar_test.json");
		fout << testpssm;
	}

	const char* test_pssm_jaspar() {
		static const char * c = ">MA0001.1 AGL3\nA  [ 0  3 79 40 66 48 65 11 65  0 ]\nC  [94 75  4  3  1  2  5  2  3  3 ]\nG  [ 1  0  3  4  1  0  5  3 28 88 ]\nT  [ 2 19 11 50 29 47 22 81  1  6 ]";
		return c;
	}

	const char* test_pssm_json () {
		static const char * c = "{"
				"	\"SERIAL_VERSIONID\" : \"Datatypes::Motifs::PSSM:1\","
				"	\"accession\" : \"MA0001.1\","
				"	\"background\" : [ 0.30, 0.20, 0.20, 0.30 ],"
				"	\"character_translation\" : \"ACGT\","
				"	\"length\" : 10,"
				"	\"matrix\" : "
				"	["
				"		0.0,"
				"		0.9690721649484536,"
				"		0.01030927835051546,"
				"		0.02061855670103093,"
				"		0.03092783505154639,"
				"		0.7731958762886598,"
				"		0.0,"
				"		0.1958762886597938,"
				"		0.8144329896907216,"
				"		0.04123711340206185,"
				"		0.03092783505154639,"
				"		0.1134020618556701,"
				"		0.4123711340206185,"
				"		0.03092783505154639,"
				"		0.04123711340206185,"
				"		0.5154639175257731,"
				"		0.6804123711340206,"
				"		0.01030927835051546,"
				"		0.01030927835051546,"
				"		0.2989690721649484,"
				"		0.4948453608247423,"
				"		0.02061855670103093,"
				"		0.0,"
				"		0.4845360824742268,"
				"		0.6701030927835051,"
				"		0.05154639175257732,"
				"		0.05154639175257732,"
				"		0.2268041237113402,"
				"		0.1134020618556701,"
				"		0.02061855670103093,"
				"		0.03092783505154639,"
				"		0.8350515463917526,"
				"		0.6701030927835051,"
				"		0.03092783505154639,"
				"		0.2886597938144330,"
				"		0.01030927835051546,"
				"		0.0,"
				"		0.03092783505154639,"
				"		0.9072164948453608,"
				"		0.06185567010309279"
				"	],"
				"	\"name\" : \"AGL3\","
				"	\"nsites\" : 97,"
				"	\"offset\" : 0,"
				"	\"pseudocount\" : 0.25"
				"}"
				"";
		return c;
	}
};


