/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __Pssm_multiplicative_H__
#define __Pssm_multiplicative_H__

template <class string, size_t alphasize = 4 >
class PSSM_Multiplicative_Score : public PSSM_Score<string> {
public:
	typedef PSSM<alphasize> _PSSM;

	/**
	 * Construct a score from a pssm and an entropy scaling factor
	 * 
	 * (default for this factor is 1, making values between 0 and 
	 *  log(4) = ca. 0.6)
	 *  
	 */
	PSSM_Multiplicative_Score (_PSSM const & _pssm, typename _PSSM::pseudocount_t t = _PSSM::PSEUDOCOUNT_BIFA) {
		pssm = _pssm;
		pssm.add_pseudocount(t);
		min_sc = min_score();
		n_sc = max_score() - min_sc;
	}

	/**
	 * A score must have a minimum and maximum for a given PSSM
	 */
	double min_score () {
		using namespace std;
		double best_score;
		double score = 0;

		for(int i = pssm.begin(); i <= pssm.end();i++){
			best_score = DBL_MAX;
			for(int j = 0; j < _PSSM::ALPHASIZE ; j++) {
				best_score = min( pssm(i, j), best_score );
			}

			// technically, this should not happen since we have added
			// pseudocounts.
			if (best_score <= DBL_EPSILON) {
				score = DBL_EPSILON;
			}

			score += log (4 * best_score);
		}

		return score;
	}

	double max_score () {
		using namespace std;
		double best_score;
		double score = 0;

		for(int i = pssm.begin(); i <= pssm.end();i++){
			best_score = 0;
			for(int j = 0; j < _PSSM::ALPHASIZE ; j++) {
				best_score = max( pssm(i, j), best_score );
			}

			if (best_score <= DBL_EPSILON) {
				score = DBL_EPSILON;
			}

			score += log (4 * best_score);
		}
		
		return score;
	}

	/**
	 * Score PSSM on one location in a given sequence
	 * 
	 * @param pssm     : the matrix
	 * @param sequence : a sequence (template class must support operator [] which returns something int)
	 * @param offset   : where to start scoring in the sequence
	 * 
	 */
	double score (string const & sequence, int offset = 0) {
		int i = pssm.begin ();
		double sc = 0;
		int any_matched = false;
		for (int j = offset; j < offset + pssm.get_length(); ++j) {
			int ch = (int)sequence[j];
			if (ch < alphasize) {
				double psc = pssm(i, (int)sequence[j]);
				if (psc < DBL_EPSILON) {
					psc = DBL_EPSILON;
				}
				sc += log (4 * psc);
				any_matched = true;
			}
			++i;
		}
		if (!any_matched) {
			return 0;
		}
		if (sc < min_sc) {
			return 0;
		}
		
		return (sc - min_sc) / n_sc;
	}

	/**
	 * Minimum sequence length for scoring
	 */
	size_t min_length () {
		return pssm.get_length();
	}

private:
	double min_sc;
	double n_sc;

	_PSSM pssm;
};


#endif // __Pssm_scoring_H__

