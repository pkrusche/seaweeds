/***************************************************************************
*   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
*   peter@dcs.warwick.ac.uk                                               *
***************************************************************************/

#ifndef __DP_MATRIX_INTEGER_H__
#define __DP_MATRIX_INTEGER_H__

#include <xasmlib/IntegerVector.h>

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
	*   UINT64 -- type of dp matrix elements 
	*   operator_t -- dp operator
	*        UINT64 operator() (size_t i, size_t j,
	*                    input_t :: element_type top, 
	*                    input_t :: element_type left,
	*                    UINT64 const & left,
	*                    UINT64 const & top_left,
	*                    UINT64 const & top,
	*        )
	* 
	* \author Peter Krusche
	* \date October 2010
	*/
	template <class input_t, class operator_t, size_t bpc>
	class dp_matrix_integer_solver {
	public:
		dp_matrix_integer_solver() {
			cur_top_row = &top_dp_row;
			cur_left_col = &left_dp_column;
			prev_top_row = cur_top_row;
			prev_left_col = cur_left_col;
		}

		utilities::IntegerVector<bpc> * cur_top_row;
		utilities::IntegerVector<bpc> * prev_top_row;
		utilities::IntegerVector<bpc> * cur_left_col;
		utilities::IntegerVector<bpc> * prev_left_col;

	private:
		/*!
		* The row and column containing the initial values of the current block
		*/
		utilities::IntegerVector<bpc> left_dp_column;
		utilities::IntegerVector<bpc> top_dp_row;

		utilities::IntegerVector<bpc> tmp_dp;

		/*!
		* the operator. 
		*/
		operator_t op;

	public:
		UINT64 operator() (input_t left_input, input_t top_input) {
			size_t m = left_input.size();
			size_t n = top_input.size();

			if(left_dp_column.size() < m+1) {
				left_dp_column.resize(m+1);
			}

			if(top_dp_row.size() < n+1) {
				top_dp_row.resize(n+1);
			}

			register UINT64 last;

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

