/***************************************************************************
 *   Copyright (C) 2009 by Peter Krusche                                   *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef MultiSeaweeds_h__
#define MultiSeaweeds_h__
#include <vector>

namespace seaweeds {

	template <class scorematrix>
	class MultiSeaweeds {
	public:
		typedef typename scorematrix::string string;
		typedef typename scorematrix::archive scorearchive;

		MultiSeaweeds() {}
		~MultiSeaweeds() {}

		/**
		* \brief compute highest score matrices for one string y and multiple strings x
		*/
		void run(const string * x, unsigned int k, const string & y) {
			p_outputs.resize(k);
			for (int _k = 0; _k < k; ++_k) {
				scorematrix sm(x[_k].size(), y.size());
				sm.semilocallcs(x[_k], y);
				p_outputs[_k] = sm.get_archive();
			}
		}

		/**
		* \brief return the seaweed permutation for a given input string after run() has been called
		*/
		scorearchive & get_seaweedpermutation(int k) {
			return p_outputs[k];
		}

	private:
		std::vector< scorearchive > p_outputs; ///< output permutations
	};

};




#endif // MultiSeaweeds_h__
