/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include "config.h"

#include <iostream>
#include <math.h>
#include <list>
#include <vector>
#include <map>

#define _RT_CUTOFF 100
#define _VERBOSE

#include "tuning/Tuning.h"
#include "xasmlib/functors.h"
#include "rangesearching/RangeBenchmark.h"
#include "rangesearching/RangeList.h"
#include "rangesearching/BinTree.h"
#include "rangesearching/RangeTree.h"
#include "rangesearching/Range2D.h"


int main(int, char**) {
	using namespace std;
	using namespace rangesearching;

	int t = 0;
	utilities::warmup();

	cout << Tuning<
			RangeBenchmark<Point2D<int>, size_t, RangeList<Point2D<int>, pair_second_less<int,int>, functors::count<Point2D<int> >, false > >,
			RangeBenchmark<Point2D<int>, size_t, RangeTree<Point2D<int>, pair_second_less<int,int>, functors::count<Point2D<int> > > >,
			int
	>::tune (0.5, t) << endl;

	return EXIT_SUCCESS;
}
