/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __NWALIGNMENT_H__
#define __NWALIGNMENT_H__

namespace alignment {

template <class _string = std::string, typename _score=int>
class NWAlignment {
	_score operator() (_string x, _string y,
			_score * rightline,
			_score * bottomline) {
		size_t m = x.size();
		size_t n = y.size();
		if( rightline == NULL ) {
			rightline = new score [m+1];
			memset(rightline, 0, sizeof(score)*(x.size()+1));
		}
		if(bottomline == NULL) {
			rightline = new score [n+1];
			memset(bottomline, 0, sizeof(score)*(y.size()+1));
		}

		for(size_t j = 0; j < n; ++j) {
			for(size_t i = 0; i < m; ++i) {
				
			}
		}
	}
};

};

#endif

