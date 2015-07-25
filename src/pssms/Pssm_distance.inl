/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

/** This was ported from the APPLES PSSM Scoring code by Alex Jironkin */

#ifndef __Pssm_distance_H__
#define __Pssm_distance_H__

/**
	*   Method for obtaining the Hellinger distance, alternative to Kullback Leibler distance,
	*   between the two matrices. See: for more details. The distance for each nucleotide
	*   distribution is added together so the sum of all alphasize distances is returned.
	*
	* @param const PSSM &pssmA A pointer to the PSSM struct for the first matrix.
	* @param const PSSM &pssmB A pointer to the PSSM struct for the second matrix.
	* @param reverse An int specifying if reverse strand should be computed.
	* 	0 - current strand, 1 - reverse strand.
	*
	* @return Returns distance between the 2 specified matrices.
	*/
template <bool reverse = false, size_t alphasize = 4>
class HellingerDistance {
public:
	double operator () (const PSSM<alphasize> & pssmA, const PSSM<alphasize> & pssmB) const {
		using namespace std;
		double result[alphasize];

		memset (result, 0, sizeof(double) * alphasize);
		int start = min (
			pssmA.begin(), pssmB.begin()
			);
		int end = max (
			pssmA.end(), pssmB.end()
			);

		for(int i = start; i <= end; i++) {
			for (int j = 0; j < alphasize; ++j) {
				double tempSqrt = 0;
				if(reverse) {
					tempSqrt = sqrt(pssmA(pssmA.reverse_pos(i), pssmA.reverse_char(j))) - sqrt(pssmB(i, j));
				} else {
					tempSqrt = sqrt(pssmA(i, j)) - sqrt(pssmB(i, j));
				}

				result[j] += 0.5*tempSqrt*tempSqrt;
			}
		}
		double res = 0;
		for (int j = 0; j < alphasize; ++j) {
			res+= sqrt (result[j]);
		}
		return res;
	}
};

/**
 * Alternative to Hellinger, a Kullback Leibler distance.
 *
 * @param const PSSM *pssmA A pointer to the PSSM struct for the first matrix.
 * @param const PSSM *pssmB A pointer to the PSSM struct for the second matrix.
 * @param bool reverse specifies if reverse strand distance should be computed.
 * 	
 * 	
 * This distance is not symmetric, dist (A, B) may not be equal to dist (B, A)
 *
 * @return Returns distance between the 2 specified matrices.
 */
template <bool reverse = false, size_t alphasize = 4>
class KullbackLeiblerDistance {
public:
	double operator () (const PSSM<alphasize> & pssmA, const PSSM<alphasize> & pssmB) const {
		using namespace std;
		
		int i;
		double result[alphasize];
		static double log2 = log(2.0);
		double res = 0;

		memset (result, 0, sizeof(double) * alphasize);

		int start = min (
			pssmA.begin(), pssmB.begin()
			);
		int end = max (
			pssmA.end(), pssmB.end()
		);

		for(i = start; i <= end; i++) {
			for (int j = 0; j < alphasize; ++j) {
				double pi = 0, qi = 0;
				if(reverse) {
					pi = pssmA(pssmA.reverse_pos(i), pssmA.reverse_char(j));
				} else {
					pi = pssmA(i, j);
				}

				qi = pssmB (i, j);
				double addme = 0;
				if (pi > FLT_MIN && qi > FLT_MIN) {
					addme = pi*( log( pi/qi )/log2 );
				} // TODO perhaps we want to count the number of times we couldn't compute
				  // a distance and return FLT_MAX if it's too high?
				result[j] += addme;
				res+= result[j];
			}
		}
		return res;
	}

};

/**
 * Distance average between two distance functions.
 */
template <class _d1, class _d2, size_t alphasize = 4> class AverageDistance {
public:
	double operator () (const PSSM<alphasize> & pssmA, const PSSM<alphasize> & pssmB) const {
		static _d1 d1;
		static _d2 d2;
		return ( d1 (pssmA, pssmB) + d2 (pssmA, pssmB) ) / 2.0;
	}
};

/**
 * Minimum distance of two distance functions.
 */
template <class _d1, class _d2, size_t alphasize = 4> class MinimumDistance {
public:
	double operator () (const PSSM<alphasize> & pssmA, const PSSM<alphasize> & pssmB) const {
		using namespace std;
		static _d1 d1;
		static _d2 d2;
		return min ( d1 (pssmA, pssmB) , d2 (pssmA, pssmB) );
	}
};

/**
 * Minimum distance of two distance functions.
 */
template <class _d> class Minimum {
public:
	double operator () (const _d & a, const _d & b) const {
		using namespace std;
		return min ( a, b );
	}
};

/**
 * Maximum distance of two distance functions.
 */
template <class _d1, class _d2, size_t alphasize = 4> class MaximumDistance {
public:
	double operator () (const PSSM<alphasize> & pssmA, const PSSM<alphasize> & pssmB) const {
		using namespace std;
		static _d1 d1;
		static _d2 d2;
		return max ( d1 (pssmA, pssmB) , d2 (pssmA, pssmB) );
	}
};

/**
 * Make a distance symmetric by averaging dist (A, B) and dist(b, a)
 */
template <class _d, size_t alphasize = 4> class SymmetricDistance {
public:
	double operator () (const PSSM<alphasize> & pssmA, const PSSM<alphasize> & pssmB) const {
		static _d d;
		return ( d (pssmA, pssmB) + d (pssmB, pssmA) ) / 2.0;
	}
};

/**
 * Combine all relative shifts of a PSSM to a combined distance
 */
template <class d, template <class> class combine, size_t alphasize = 4 > class AllRotationsDistance {
public:
	double operator () (const PSSM<alphasize> & pssmA, const PSSM<alphasize> & pssmB) const {
		PSSM<alphasize> shifted_A = pssmA;
		PSSM<alphasize> shifted_B = pssmB;
		shifted_A.reset_shift().shift_left (pssmA.get_length() - 1);
		shifted_B.reset_shift();

		static d distance;
		static combine<double> combiner;
		
		double current_distance = DBL_MAX;
		bool first = true;

		while (shifted_A.begin () <= shifted_B.end()) {
			if (first) {
				first = false;
				current_distance = distance (shifted_A, shifted_B);
			} else {
				current_distance = 
					combiner ( current_distance, 
							   distance (shifted_A, shifted_B) 
							 );
			}
			shifted_A.shift_right(1);
		}
		return current_distance;
	}
};

typedef AllRotationsDistance < 
		SymmetricDistance <
			MinimumDistance < KullbackLeiblerDistance < false >, KullbackLeiblerDistance < true > >
		>
		, Minimum
		>
	Both_Strands_DNA_Min_KullbackLeibler_Distance;

typedef AllRotationsDistance < 
		SymmetricDistance <
		KullbackLeiblerDistance < false >
		>
		, Minimum
	>
	Same_Strand_DNA_Min_KullbackLeibler_Distance;

typedef AllRotationsDistance < 
		SymmetricDistance <
		KullbackLeiblerDistance < true >
		>
		, Minimum
	>
	Opposite_Strand_DNA_Min_KullbackLeibler_Distance;

typedef AllRotationsDistance < 
	SymmetricDistance <
	MinimumDistance < HellingerDistance < false >, HellingerDistance < true > >
	>
	, Minimum
>
Both_Strands_DNA_Min_Hellinger_Distance;

typedef AllRotationsDistance < 
	SymmetricDistance <
	HellingerDistance < false >
	>
	, Minimum
>
Same_Strand_DNA_Min_Hellinger_Distance;

typedef AllRotationsDistance < 
	SymmetricDistance <
	HellingerDistance < true >
	>
	, Minimum
>
Opposite_Strand_DNA_Min_Hellinger_Distance;


#endif // __Pssm_distance_H__
