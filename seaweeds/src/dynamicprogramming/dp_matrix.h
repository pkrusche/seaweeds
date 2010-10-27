/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __DP_MATRIX_H__
#define __DP_MATRIX_H__

#include <vector>

namespace dynamic_programming {

	/*!
	 * \class dp_matrix_rowmajor_solver
	 *
	 * \brief class to solve dynamic programming problems generically
	 * 
	 * Will compute all elements of the dynamic programming matrix.
	 * 
	 * template class requirements:
	 *
	 *   input_t -- array/vector type
	 *        operator() (int) returning some type that will be passed to
	 *                         operator_t :: operator ()
	 *        size_t size() returning the size of the input
	 *   dp_element_t -- type of dp matrix elements 
	 *   operator_t -- dp operator
	 *        operator() (size_t i, size_t j,
	 *                    input_t :: element_type top, 
	 *                    input_t :: element_type left,
	 *                    dp_element_t const & left,
	 *                    dp_element_t const & top_left,
	 *                    dp_element_t const & top, 
	 *                    dp_element_t & current
	 *        )
	 * 
	 * \author Peter Krusche
	 * \date October 2010
	 */
	template <class input_t, class dp_element_t, class operator_t>
	class dp_matrix_solver {
	public:
		/*!
		 * The row and column containing the initial values of the current block
		 */
		std::vector<dp_element_t> left_dp_column;
		std::vector<dp_element_t> top_dp_row;

		/*!
		 * the operator. 
		 */
		operator_t op;

		void operator() (input_t left_input, input_t top_input) {
			size_t m = left_input.size();
			size_t n = top_input.size();

			if(left_dp_column.size() < m+1) {
				left_dp_column.resize(m+1)
			}

			if(top_dp_row.size() < n+1) {
				top_dp_row.resize(n+1)
			}
			
			if(m >= n) {
				for (size_t i = 1; i <= m; ++i) {
					dp_element_t left = left_dp_column[i];
					for (size_t j = 1; j <= n; ++j) {
						dp_element_t left_copy = left;
						op(i, j, left_input[i-1], top_input[j-1], left_copy, top_dp_row[j-1], top_dp_row[j], left);
						top_dp_row[j] = left;
					}
					left_dp_column[i] = left;
				}
			} else {
				for (size_t j = 1; j <= n; ++j) {
					dp_element_t top = top_dp_row[j];
					for (size_t i = 1; i <= m; ++i) {
						dp_element_t top_copy = top;
						op(i, j, left_input[i-1], top_input[j-1], left_dp_column[i], left_dp_column[i-1], top_copy, top);
						left_dp_column[i] = top;
					}
					top_dp_row[j] = top;
				}
			}
		}
	};
};

#endif

