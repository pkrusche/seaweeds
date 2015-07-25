/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <sstream>

#include "UnitTest++.h"

#include "datamodel/TextIO.h"
#include "datamodel/SequenceTranslation.h"

using namespace UnitTest;


namespace {
	TEST(Test_TextIO_Split_And_Trim)
	{
		using namespace std;

		vector<string> test2;
		TextIO::split (string ("a, b , c, d"), test2, ",");
  
		for (vector<string>::iterator it = test2.begin(), end = test2.end(); it != end; ++it) {
			*it = TextIO::trim(*it);
		}
		CHECK_EQUAL("a", test2[0]);
		CHECK_EQUAL("b", test2[1]);
		CHECK_EQUAL("c", test2[2]);
		CHECK_EQUAL("d", test2[3]);
	}

	TEST(Test_SequenceStreamsReading) {
		utilities::init_xasmlib();
		using namespace std;
		using namespace utilities;

		stringstream s;
		string test =  "ACGATATACTATA";
		string test2 = "ACGATATACNATA";
		string test3 = "CACGTGCACGTGC";
		string test_out;

		IntegerVector <2> testvec_1;

		s << test << test2 << test3;
		
		datamodel::TranslatingInputStream<2> stream (s, "ACGT");

		testvec_1.resize(test.size());
		stream.read_sequence (testvec_1);
		
		CHECK_EQUAL (test, datamodel::unwrap_sequence<2>(testvec_1, "ACGT", 'N'));
		
		CHECK_THROW ( stream.read_sequence (testvec_1), runtime_error );

		stream.read_sequence (testvec_1);
		CHECK_EQUAL (test3, datamodel::unwrap_sequence<2>(testvec_1, "ACGT", 'N'));
	}


	TEST(Test_SequenceStreamsWindows) {
		utilities::init_xasmlib();
		using namespace std;
		using namespace utilities;

		stringstream s;
		string test =  "ACGATATACTATACGATATACNATACACGTGCACGTGC";
		s << test;
		
		datamodel::TranslatingInputStream<8> stream (s, "ACGTN");
		
		datamodel::WindowedInputStream<8> winstr (3);
		winstr.set_input ( stream );
		
		int position = -1;
		test+= "XXX";
		for (int j = 0; j < test.size() - 3; ++j) {
			IntegerVector <8> testvec (4);
			size_t read = winstr.get_window(4, testvec, position);
			if (j < (int)test.size() - 2 - 4) {
				CHECK_EQUAL(4, read);
			} else {
				CHECK_EQUAL((int)test.size() - 4 - j + 1, read);
			}
			
			CHECK_EQUAL(j, position);
			CHECK_EQUAL(4, testvec.size());
			CHECK_EQUAL(test.substr(j, 4), datamodel::unwrap_sequence<8>(testvec, "ACGTN", 'X'));
		}
	}

	TEST(Test_SequenceStreamsWindows2) {
		utilities::init_xasmlib();
		using namespace std;
		using namespace utilities;

		stringstream s;
		string test =  "ACGATATACTATACGATATACNATACACGTGCACGTGCACGATATACTATACGATATACNATACACGTGCACGTGCACGATATACTATACGATATACNATACACGTGCACGTGC";
		s << test;
		
		datamodel::TranslatingInputStream<8> stream (s, "ACGTN");
		
		datamodel::WindowedInputStream<8> winstr (11);
		winstr.set_input ( stream );
		
		int position = -1;
		test+= "XXXXXXXXXXXXXXX";
		for (int j = 0; j*4 + 15 < test.size(); ++j) {
			IntegerVector <8> testvec (15);
			size_t read = winstr.get_window(15, testvec, position);
			if (4*j + 15 < (int)test.size() - 15) {
				CHECK_EQUAL(15, read);
			} else {
				CHECK_EQUAL(max (0, (int)test.size() - 15 - 4*j), read);
			}
			
			CHECK_EQUAL(j*4, position);
			CHECK_EQUAL(15, testvec.size());
			CHECK_EQUAL(test.substr(j*4, 15), datamodel::unwrap_sequence<8>(testvec, "ACGTN", 'X'));
		}
	}

	TEST(Test_SequenceStreamsWindows3) {
		utilities::init_xasmlib();
		using namespace std;
		using namespace utilities;

		stringstream s;
		string test =  "ACGATATACTATAACGATATACTATACGATATACNATACACGTGCACGTGC";
		s << test;
		
		datamodel::TranslatingInputStream<8> stream (s, "ACGTN");		
		datamodel::WindowedInputStream<8> winstr (20);
		winstr.set_input ( stream );
		
		int position = -1;
		IntegerVector <8> testvec (15);
		size_t read = winstr.get_window(512, testvec, position);
		CHECK_EQUAL(test.size(), read);
		CHECK_EQUAL(0, position);
		testvec.resize(read);
		CHECK_EQUAL(test, datamodel::unwrap_sequence<8>(testvec, "ACGTN", 'X'));

		read = winstr.get_window(512, testvec, position);
		CHECK_EQUAL(0, read);
		CHECK_EQUAL(512 - 20, position);

		read = winstr.get_window(512, testvec, position);
		CHECK_EQUAL(0, read);
		CHECK_EQUAL(1024 - 20 - 20, position);
	}


	TEST(Test_Transliterate) {
		utilities::init_xasmlib();
		using namespace std;
		using namespace utilities;
		using namespace datamodel;

		string test =   "ACGATATACTATACGATATACNATACACGTGCACGTGC";
		string rtest =  "TGCTATATGATATGCTATATGNTATGTGCACGTGCACG";

		IntegerVector <4> test1 = make_sequence<4>(test.c_str(), "ACGTN");
		IntegerVector <4> test1a;

		transliterate_string<string, 4> (test, test.size(), test1a, "ACGTN");
		CHECK_EQUAL (test, unwrap_sequence<4>(test1, "ACGTN", 'X'));
		CHECK_EQUAL (test, unwrap_sequence<4>(test1a, "ACGTN", 'X'));

		IntegerVector <5> test2;

		transliterate<4,5>(test1, test2, "ACGTN", "TGCAN");

		CHECK_EQUAL (test.size(), test1.size());
		CHECK_EQUAL (rtest.size(), test2.size());
		CHECK_EQUAL (test, unwrap_sequence<4>(test1, "ACGTN", 'X'));
		CHECK_EQUAL (rtest, unwrap_sequence<5>(test2, "ACGTN", 'X'));
	}

	TEST(Test_ReverseComplement) {
		utilities::init_xasmlib();
		using namespace std;
		using namespace utilities;
		using namespace datamodel;

		string test =   "ACGNATATACTATACGATATACATACACGTGCACGTGC";
		string rtest =  "GCACGTGCACGTGTATGTATATCGTATAGTATATNCGT";

		IntegerVector <8> test1 = make_sequence<8>(test.c_str(), "ACGTN");
		CHECK_EQUAL (test, unwrap_sequence<8>(test1, "ACGTN", 'X'));
		reverse_complement<8> (test1, 4);
		CHECK_EQUAL (rtest, unwrap_sequence<8>(test1, "ACGTN", 'X'));
	}

};
