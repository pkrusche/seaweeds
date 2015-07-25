/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

#include "UnitTest++.h"

#include "apps/Seaweeds/AlignmentPlot.h"
#include "apps/Seaweeds/AlignmentPlotIO.h"

using namespace UnitTest;

#define TEST_SIZE_X 50
#define TEST_SIZE_Y 42

#define TEST_W 20

#define TEST_M (TEST_SIZE_X + TEST_W - 1)
#define TEST_N (TEST_SIZE_Y + TEST_W - 1)

#define TEST_OFFSET_X 10
#define TEST_OFFSET_Y 20

namespace {
	/** randomized alignment plot data */
	double ap_data [TEST_SIZE_X][TEST_SIZE_Y];

	int ixs[TEST_SIZE_X*TEST_SIZE_Y];

	double all_ap_scores[TEST_SIZE_X*TEST_SIZE_Y];

	double ap_row_maxima[TEST_SIZE_X];

	double ap_col_maxima[TEST_SIZE_Y];

	// default bucketing, if this is changed in the AlignmentPlot constructor, this will stop
	// working...!
	int hist_buckets[1000];

	struct ix_sort {
		bool operator()(int ix1, int ix2) {
			return all_ap_scores[ix1] < all_ap_scores[ix2];
		}
	};

	void init_ap_data() {
		using namespace std;
		int k = 0;
		memset(hist_buckets, 0, sizeof(int)*1000);
		for (int j = 0; j < TEST_SIZE_Y; ++j) {
			ap_col_maxima[j] = 0;
		}
		for (int i = 0; i < TEST_SIZE_X; ++i) {
			ap_row_maxima[i] = 0;
			for (int j = 0; j < TEST_SIZE_Y; ++j) {
				ap_data[i][j] = ((double)rand()) / RAND_MAX;
				
				ap_row_maxima[i] = max (ap_row_maxima[i], ap_data[i][j]);
				ap_col_maxima[j] = max (ap_col_maxima[j], ap_data[i][j]);
				ixs[k] = k;
				all_ap_scores[k++] = ap_data[i][j];
				size_t b = (size_t)(ap_data[i][j] * 1000);
				if(b >= 1000) {
					b = 1000;
				}
				hist_buckets[b]++;
			}
		}
		std::sort(&ixs[0], &ixs[TEST_SIZE_X*TEST_SIZE_Y], ix_sort());
	}

	void sample_ap (AlignmentPlot & ap, 
			int x0 = 0, int x1 = TEST_SIZE_X, int y0 = 0, int y1 = TEST_SIZE_Y) {
		for (int i = x0; i < x1; ++i) {
			for (int j = y0; j < y1; ++j) {
				windowlocal::window w(i-TEST_OFFSET_X, j-TEST_OFFSET_Y, ap_data[i][j]);

				ap.report_score(w);
			}
		}
	}

	class translateme : public windowlocal::window_translator {
	public:
		bool translate(windowlocal::window & w) {
			w.x0+= TEST_OFFSET_X;
			w.x1+= TEST_OFFSET_Y;

			return true;
		}
	};

	struct window_sorter {
		bool operator() (windowlocal::window const & l, windowlocal::window const & r) {
			return l.score < r.score;
		}
	};

	void check_ap(AlignmentPlot & ap) {
		using namespace std;
		/** check profiles */
		std::vector<double> v;
		ap.get_profile_a(v);
		CHECK_EQUAL(TEST_SIZE_X, v.size());
		for (int i = 0; i < TEST_SIZE_X; ++i) {
			CHECK_CLOSE( ap_row_maxima[i], v[i], 0.0000001 );
		}

		v.clear();
		ap.get_profile_b(v);
		CHECK_EQUAL(TEST_SIZE_Y, v.size());
		for (int i = 0; i < TEST_SIZE_Y; ++i) {
			CHECK_CLOSE( ap_col_maxima[i], v[i], 0.0000001 );
		}

		/** check sampled windows */
		std::vector<windowlocal::window> vw;
		ap.get_windows(vw);
		CHECK(TEST_SIZE_X*TEST_SIZE_Y >= vw.size());
		CHECK(vw.size() > max(TEST_SIZE_X, TEST_SIZE_Y));

		std::sort(vw.begin(), vw.end(), window_sorter());
		for (int _i = 0; _i < vw.size(); ++_i)	{
			int i = _i;
			int ix = ixs[TEST_SIZE_X*TEST_SIZE_Y - 1 - (vw.size() - i - 1)];

			bool found = false;
			while(i < vw.size() && fabs(all_ap_scores[ix] - vw[i].score) < 0.0000001 ) {
				int ix0 = ix / TEST_SIZE_Y;
				int ix1 = ix % TEST_SIZE_Y;

				if(ix0 == vw[i].x0 && ix1 == vw[i].x1) {
					found = true;
					break;
				}

				++i;
				if (i < vw.size()) {
					ix = ixs[TEST_SIZE_X*TEST_SIZE_Y - 1 - (vw.size() - i - 1)];
				}
			}

			CHECK(found);
		}

		/** check the histogram */
		utilities::Histogram & h(ap.get_score_histogram());
		CHECK_EQUAL(1000, h.nbuckets);
		CHECK_CLOSE(h.max_val, 1, 0.0000001);
		CHECK_CLOSE(h.min_val, 0, 0.0000001);
		CHECK_EQUAL(0, h.buckets[0]);	// no under/overflow should happen
		CHECK_EQUAL(0, h.buckets[1]);	// no under/overflow should happen
		for (int i = 2; i < 1002; ++i)	{
			CHECK_EQUAL(hist_buckets[i-2], h.buckets[i]);
		}
	}

	TEST(Test_AligmentPlot_Sampling) {
		init_ap_data();

		AlignmentPlot ap (TEST_M, TEST_N, TEST_W);

		/** we test if the coordinates are shifted correctly by specifying 
		 *  an offset, and then shifting them back in the translation step */
		ap.set_translator<translateme>();

		sample_ap(ap);

		check_ap(ap);
	}

	TEST(Test_AligmentPlot_Assign) {
		init_ap_data();

		AlignmentPlot ap (TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap2;

		/** we test if the coordinates are shifted correctly by specifying 
		 *  an offset, and then shifting them back in the translation step */
		ap.set_translator<translateme>();

		sample_ap(ap);

		ap2 = ap;

		check_ap(ap2);
	}

	TEST(Test_AligmentPlot_Serialize) {
		init_ap_data();

		AlignmentPlot ap (TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap2;

		/** we test if the coordinates are shifted correctly by specifying 
		 *  an offset, and then shifting them back in the translation step */
		ap.set_translator<translateme>();

		sample_ap(ap);

		size_t sz = ap.serialized_size();
		char * data = new char[sz];
		ap.serialize(data, sz);
		ap2.deserialize(data, sz);
		delete [] data;

		check_ap(ap2);
	}

	TEST(Test_AligmentPlot_Combine_V) {
		init_ap_data();

		AlignmentPlot ap1(TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap2(TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap3(TEST_M, TEST_N, TEST_W);
		AlignmentPlot apr(TEST_M, TEST_N, TEST_W);

		/** we test if the coordinates are shifted correctly by specifying 
		 *  an offset, and then shifting them back in the translation step */
		ap1.set_translator<translateme>();
		ap2.set_translator<translateme>();
		ap3.set_translator<translateme>();

		// Three parts vertical
		sample_ap(ap1, 0, TEST_SIZE_X/3);
		sample_ap(ap2, TEST_SIZE_X/3, 2*TEST_SIZE_X/3);
		sample_ap(ap3, 2*TEST_SIZE_X/3);
		apr.reduce_with(&ap1);
		apr.reduce_with(&ap2);
		apr.reduce_with(&ap3);

		check_ap(apr);
	}

	TEST(Test_AligmentPlot_Combine_H) {
		init_ap_data();

		AlignmentPlot ap1(TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap2(TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap3(TEST_M, TEST_N, TEST_W);
		AlignmentPlot apr(TEST_M, TEST_N, TEST_W);

		/** we test if the coordinates are shifted correctly by specifying 
		 *  an offset, and then shifting them back in the translation step */
		ap1.set_translator<translateme>();
		ap2.set_translator<translateme>();
		ap3.set_translator<translateme>();

		// Three parts horizontal
		sample_ap(ap1, 0, TEST_SIZE_X, 0, TEST_SIZE_Y/3);
		sample_ap(ap2, 0, TEST_SIZE_X, TEST_SIZE_Y/3, 2*TEST_SIZE_Y/3); 
		sample_ap(ap3, 0, TEST_SIZE_X, 2*TEST_SIZE_Y/3, TEST_SIZE_Y); 
		apr.reduce_with(&ap1);
		apr.reduce_with(&ap2);
		apr.reduce_with(&ap3);

		check_ap(apr);
	}

	TEST(Test_AligmentPlot_Combine_Q) {
		init_ap_data();

		AlignmentPlot ap1(TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap2(TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap3(TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap4(TEST_M, TEST_N, TEST_W);
		AlignmentPlot apr(TEST_M, TEST_N, TEST_W);

		/** we test if the coordinates are shifted correctly by specifying 
		 *  an offset, and then shifting them back in the translation step */
		ap1.set_translator<translateme>();
		ap2.set_translator<translateme>();
		ap3.set_translator<translateme>();
		ap4.set_translator<translateme>();

		// Three parts horizontal
		sample_ap(ap1, 0, TEST_SIZE_X/2,              0,             TEST_SIZE_Y/2);
		sample_ap(ap2, 0, TEST_SIZE_X/2,              TEST_SIZE_Y/2, TEST_SIZE_Y); 
		sample_ap(ap3, TEST_SIZE_X/2, TEST_SIZE_X,    0,             TEST_SIZE_Y/2);
		sample_ap(ap4, TEST_SIZE_X/2, TEST_SIZE_X,    TEST_SIZE_Y/2, TEST_SIZE_Y); 

		apr.reduce_with(&ap1);
		apr.reduce_with(&ap2);
		apr.reduce_with(&ap3);
		apr.reduce_with(&ap4);

		check_ap(apr);
	}

	TEST(Test_AligmentPlot_IO_Assign) {
		init_ap_data();

		AlignmentPlot ap (TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap2;

		/** we test if the coordinates are shifted correctly by specifying 
		 *  an offset, and then shifting them back in the translation step */
		ap.set_translator<translateme>();

		sample_ap(ap);

		AlignmentPlotIO io;

		io.init_from(ap);

		io.assign_to(ap2);

		check_ap(ap2);

	}

	TEST(Test_AligmentPlot_IO_RW) {
		init_ap_data();

		AlignmentPlot ap (TEST_M, TEST_N, TEST_W);
		AlignmentPlot ap2;

		ap.set_translator<translateme>();

		sample_ap(ap);

		AlignmentPlotIO io;

		io.init_from(ap);

		std::string data;

		{
			std::ostringstream os;
			os << io;
			data = os.str();
		}

		AlignmentPlotIO io2;
		{
			std::istringstream is(data);
			is >> io2;
		}

		io2.assign_to(ap2);

		check_ap(ap2);

	}

};
