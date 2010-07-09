/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef ATI_Gpulib_h__
#define ATI_Gpulib_h__

#include <vector>

#include "brook/Stream.h" 
#include "xasmlib/IntegerVector.h"
#include "bspcpp/tools/aligned_allocator.h"

namespace seaweeds {

	class MultiSeaweeds_GPU {
	public:
		typedef utilities::IntegerVector<8> string;

		MultiSeaweeds_GPU(int _y_chunksize = 512);
		~MultiSeaweeds_GPU();

		/**
		 * \brief compute highest score matrices for one string y and multiple strings x
		 */
		void run(const string * x, unsigned int k, const string & y);

		/**
		 * \brief return the seaweed permutation for a given input string after run() has been called
		 */
		std::vector<int> & get_seaweedpermutation(int k);

	private:
		unsigned int m, n;	///< the sizes of the generated dummy strings
		unsigned int y_chunksize; ///< size of chunks into which we split string y
		std::vector< std::vector<int> > p_outputs; ///< output permutations
		utilities::aligned_allocator<int, 256> alloc;	///< our allocator
	};

};

#include "seaweeds.cpp"


#endif // ATI_Gpulib_h__

