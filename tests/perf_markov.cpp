/***************************************************************************
 *   Copyright (C) 2011   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#include "autoconfig.h"

#include "bsp.h"

#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "datamodel/SequenceTranslation.h"
#include "sequencemodel/MarkovModel.h"

#define ORDER 3

int main (int argc, char ** argv) {
	using namespace std;
	using namespace utilities;
	using namespace datamodel;
	using namespace sequencemodel;

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

	// generate a lot of chars
	int N = (int )1e8;
	{ // benchmark generation
		bsp_warmup(2);

		double t0 = bsp_time();
		IntegerVector<2> test2 = model.generate_sequence(N);
		double t1 = bsp_time();
		test2.zero();
		double t2 = bsp_time();

		cout << "Time for generating " << N << " bases: " << t1 - t0 
			 << "s ; [plain writing time: " << t2 - t1 <<  " ] " << endl;
	}

	{ // benchmark learning
		IntegerVector<2> test3 (N);
		MarkovModel<2,2> model2;
		model2.order (ORDER);

		double t0 = bsp_time();

		for (size_t j = 0; j < N; ++j) {
			test3[j] = rand() & 3;
		}

		double t1 = bsp_time();

		model2.learn(test3);

		double t2 = bsp_time();

		cout << "Time for learning model from " << N << " bases: " << t2 - t1 
			<< "s ; [time for randomly generating these: " << t1 - t0 <<  " ] " << endl;
	}


	return 0;
}
