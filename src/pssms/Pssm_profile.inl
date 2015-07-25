/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __Pssm_profile_H__
#define __Pssm_profile_H__

#ifndef PSSM_HIST_NBUCKETS
#define PSSM_HIST_NBUCKETS 10000
#endif

#ifndef PSSM_HIST_SAMPLES
#define PSSM_HIST_SAMPLES 20000000l
#endif

/************************************************************************/
/* Score Histogram class to produce PSSM scoring p-values using         */
/* a background model                                                   */
/************************************************************************/

template <class _string>
class PSSM_Histogram : public datamodel::Serializable {
public:

	/** Operator= ... */
	PSSM_Histogram const & operator= (PSSM_Histogram const & rhs) {
		if (&rhs == this) {
			return *this;
		}
		tail_scores = rhs.tail_scores;
		
		// this needs to come last!
		this->datamodel::Serializable::operator= (rhs);
		return *this;
	}

	/*
	 * Create a histogram given a PSSM, a sequence background model, and 
	 * a score function.
	 * 
	 * We can also restrict the range of a histogram via a best and worst 
	 * right-tail p-value.
	 * 
	 * \param [ps] the score function
	 * \param [model] the sequence background model
	 * 
	 */

	void make_histogram( 
		PSSM_Score <_string> * ps,
		sequencemodel::SequenceModel< _string > * model ) {
		using namespace std;
		using namespace utilities;

		// the more accuracy we want on the pvals, the more samples
		// we need.
		size_t samples = PSSM_HIST_SAMPLES;

		tail_scores.resize( PSSM_HIST_NBUCKETS );

		for (int j = 0; j < PSSM_HIST_NBUCKETS; ++j) {
			tail_scores[j] = 0;
		}

		_string v= model->generate_sequence(samples + ps->min_length());
		for (int k = 0; k < samples; ++k) {
			double score = ps->score(v, k);
			if(score < 0 || score > 1) {
				score = -1;
			}
			tail_scores[ (size_t)(score * ((double)PSSM_HIST_NBUCKETS-1))]++;
		}

		for (int j = PSSM_HIST_NBUCKETS-2; j >= 0; --j) {
			tail_scores[j] += tail_scores[j+1];
		}
	}

	/**
	 * Return the right tail cumulative probability of the distribution, given a score.
	 * 
	 * If the score is too small, we return 1 (meaning we have probability 1 of getting a 
	 * better score than this). 
	 * 
	 * \param[min_score] the score
	 * 
	 * \return the cumulative empirical probability of scoring higher than min_score on the 
	 *         background model
	 * 
	 */

	double right_tail ( double min_score ) const  {
		if (min_score < 0) {
			return 1; 
		}
		if (min_score > 1) {
			return 0;
		}
		return ((double)tail_scores[(size_t)(min_score * (PSSM_HIST_NBUCKETS-1))]) / ((double)tail_scores[0]);
	}

	/**
	 * Return the empirical probability of scoring within a range of scores
	 * 
	 * If the score is too small, we return 1 (meaning we have probability 1 of getting a 
	 * better score than this). 
	 * 
	 * \param[min_score] the minimum score
	 * \param[max_score] the maximum score
	 * 
	 * \return the cumulative empirical probability of scoring higher than min_score, 
	 *         but lower than max_score on the background model
	 * 
	 */
	double p ( double min_score, double max_score ) const {
		ASSERT (max_score > min_score);
		return right_tail(min_score) - right_tail(max_score);
	}

	/**
	 * Return the minimum score on the background that will produce a give p-value
	 * 
	 * \param[p] The p-value
	 * 
	 * \return the minimum score on the background better than a fraction of p of all the
	 *         other scores
	 * 
	 */

	double right_tail_min_score ( double p ) const {
		ASSERT (p >  0.0);
		ASSERT (p <= 1.0);
		
		int beg = 0, end = PSSM_HIST_NBUCKETS-1;
		double score = 0;
		while (end - beg > 1) {
			int mid = (beg+end+1)/2;
			score = ((double)mid) / PSSM_HIST_NBUCKETS;
			if ( right_tail(score) < p ) {
				end = mid;
			} else {
				beg = mid;				
			}
		}
		return score;
	}
	
private:
	/************************************************************************/
	/* Serialization code                                                   */
	/************************************************************************/
	JSONIZE_AS (
		"Datatypes::Motifs::PSSM_Profile", 
		PSSM_Histogram, 1, 
		S_STORE(tail_scores, JSONArray< JSONInt<> >)
	);
	std::vector<int> tail_scores;
};


#endif // __Pssm_profile_H__