/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __LCS_H__
#define __LCS_H__

#include <stdlib.h>
#include <string.h>
#include <string>

namespace lcs {

/** compute longest common subsequences by dynamic programming */

template <class _string = std::string>
class Llcs {
public:
	typedef _string string;

	 size_t operator()(string const & _a, string const & _b, 
	 				   int * rightline  = NULL,
					   int * bottomline = NULL
	 ) {
		using namespace std;
        bool alloc_b = false;
        bool alloc_r = false;
        
        size_t al = _a.size ();
        size_t bl = _b.size ();
        if (rightline == NULL) {
            rightline = new int[bl + 1];
			memset(rightline, 0, sizeof(int)*(bl+1));
            alloc_r = true;
        }
		if (bottomline == NULL) {
            bottomline = new int[al + 1];
            alloc_b = true;
			memset(bottomline, 0, sizeof(int)*(al+1));
        }
        int *pcl = new int[al + 1];
        int *ppl = bottomline;
        int upper_right = bottomline[al];
        int lower_left = rightline[bl];
        for (size_t i = 1; i <= bl; ++i) {
            pcl[0] = rightline[i];
            for (size_t j = 1; j <= al; ++j) {
                if (_a[j - 1] == _b[i - 1]) {
                    pcl[j] = ppl[j - 1] + 1;
                } else {
                    pcl[j] = max (ppl[j], pcl[j - 1]);
                }
            }
            rightline[i] = pcl[al];
            int *temp = pcl;
            pcl = ppl;
            ppl = temp;
        } 
		int len = ppl[al];
        if (ppl != bottomline) {
            memcpy (bottomline, ppl, sizeof (int) * (al + 1));
            delete[]ppl;
        } else {
            delete[]pcl;
        }
        rightline[0] = upper_right;
        bottomline[0] = lower_left;
        if (alloc_r) {
            delete[]rightline;
        }
        if (alloc_b) {
            delete[]bottomline;
        }
        return len;
    }
};

};

#endif
