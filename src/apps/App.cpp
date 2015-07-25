/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "App.h"

#include <iostream>
#include <tbb/spin_mutex.h>
#include <tbb/atomic.h>

tbb::atomic<int> g_progress;
int g_max_progress;
int g_progress_percent;
tbb::spin_mutex g_progress_mutex;

/************************************************************************/
/* Progress output helpers                                              */
/************************************************************************/
void start_progress( const char * what, int _max ) {
	ASSERT (_max > 0);
	g_progress = 0;
	g_progress_percent = 0;
	g_max_progress = _max;
	std::cerr << what << std::endl;
	std::cerr << "[........|........|........|........|........|........|........|........]" << std::endl;
}

void add_progress (int p) {
	int pp = g_progress.fetch_and_add(p);

	p = pp*74/ (g_max_progress < 1 ?  1 : g_max_progress );
	if (p > g_progress_percent) {
		tbb::spin_mutex::scoped_lock l (g_progress_mutex);
		std::string s;
		while (g_progress_percent < p) {
			++g_progress_percent;
			s+= "#";
		}
		std::cerr << s;
	}
}

void end_progress() {
	std::cerr << "." << std::endl;
}

