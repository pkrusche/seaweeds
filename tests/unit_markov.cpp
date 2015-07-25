/***************************************************************************
 *   Copyright (C) 2011   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "UnitTest++.h"

#include "datamodel/SequenceTranslation.h"
#include "sequencemodel/MarkovModel.h"

#define ORDER 3

// #define _VERBOSETEST

using namespace std;
using namespace utilities;
using namespace datamodel;
using namespace sequencemodel;
using namespace UnitTest;

namespace {

	TEST(Test_Markov_Sequence_Models) {
		init_xasmlib();
		IntegerVector<2> test (make_sequence<2> (
			"TAGGGGAAATGGCCGCCATTGCTAAGATATTTGGCCTACTTCTAATCCCTCCGACGATGA"
			"ACCAGCATTAATTCACAACACCATGCTTTTCCTTTCATGCATTTAGCGTTTTCTCTGCAC"
			"AAAACGCGCAGTATATAATACTTAAGAACCGTCGTTGCGTTGCGTTCACGTGTCATAGGA"
			"ATGTGTGGCAGCGGTGTTTACCTCCTCGCTGCGCCCCATGACTTGTTTTGATGGCGACCA"
			"CGTGACCGACTCCGAGAGTAGGACGTCCCCGCTGTCTTTTTGAGCACATTACAAACAAGT"
			"TGCAGATAAGGGGACGAGAAGGAGTCGCCCGAGGATAGTTCCTACCGAAAACACTTCGAC"
			"GTGACGCCGCCTGCCTGCTACTATTAGTTATTA"));

		MarkovModel<2,2> model;
		model.order( ORDER );

		model.learn(test);

		// initialize rng
		model.generate_sequence(2000);

		// test serialization
		{
			ofstream model_archive ("examples/markovmodel/test_model.json");
			model_archive << model;
		}

		MarkovModel<2,2> model_copy;
		{
			ifstream in_archive("examples/markovmodel/test_model.json");
			in_archive >> model_copy;
		}

		{
			ofstream model_archive ("examples/markovmodel/test_model_copy.json");
			model_archive << model_copy;
		}

		// we want to get the same result for model and copy
		IntegerVector<2> test2 = 
			model.generate_sequence(test.size() - 20);

		test2.append( model.generate_sequence(20) );

		IntegerVector<2> test2_copy = model_copy.generate_sequence(test.size() - 20);
		test2_copy.append (model_copy.generate_sequence(20));

		CHECK_EQUAL (test.size(), test2.size());
		for (size_t j = 0; j < test2.size(); ++j) {
			CHECK_EQUAL (test2.get(j), test2_copy.get(j));
		}

#ifdef _VERBOSETEST
		cout << "input sequence  : " << datamodel::unwrap_sequence<2> ( test ) << endl;
		cout << "random sequence : " << datamodel::unwrap_sequence<2> ( test2 ) << endl;
#endif
	}

};

