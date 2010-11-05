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
	 *        dp_element_t operator() (size_t i, size_t j,
	 *                    input_t :: element_type top, 
	 *                    input_t :: element_type left,
	 *                    dp_element_t const & left,
	 *                    dp_element_t const & top_left,
	 *                    dp_element_t const & top,
	 *        )
	 * 
	 * \author Peter Krusche
	 * \date October 2010
	 */
	template <class input_t, class dp_element_t, class operator_t>
	class dp_matrix_solver {
	public:
		dp_matrix_solver() {
			cur_top_row = &top_dp_row;
			cur_left_col = &left_dp_column;
			prev_top_row = cur_top_row;
			prev_left_col = cur_left_col;
		}

		std::vector<dp_element_t> * cur_top_row;
		std::vector<dp_element_t> * prev_top_row;
		std::vector<dp_element_t> * cur_left_col;
		std::vector<dp_element_t> * prev_left_col;

	private:
		/*!
		 * The row and column containing the initial values of the current block
		 */
		std::vector<dp_element_t> left_dp_column;
		std::vector<dp_element_t> top_dp_row;

		std::vector<dp_element_t> tmp_dp;

		/*!
		 * the operator. 
		 */
		operator_t op;

	public:
		dp_element_t operator() (input_t left_input, input_t top_input) {
			size_t m = left_input.size();
			size_t n = top_input.size();

			if(left_dp_column.size() < m+1) {
				left_dp_column.resize(m+1);
			}

			if(top_dp_row.size() < n+1) {
				top_dp_row.resize(n+1);
			}
			
			register dp_element_t last;

			if(tmp_dp.size() < top_dp_row.size()) {
				tmp_dp.resize(top_dp_row.size());
			}
			prev_top_row = &top_dp_row;
			cur_top_row = &tmp_dp;
		
			for (register size_t i = 1; i <= m; ++i) {
				last = (*cur_left_col)[i];
				for (register size_t j = 1; j <= n; ++j) {
					last = op(i, j, left_input[i-1], top_input[j-1], last, (*prev_top_row)[j-1], (*prev_top_row)[j]);
					(*cur_top_row)[j] = last;
				}
				swap(cur_top_row, prev_top_row);
				(*cur_left_col)[i] = last;
			}
			return last;
		}
	};
};

#endif

