/***************************************************************************
*   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
*   peter@dcs.warwick.ac.uk                                               *
***************************************************************************/

#ifndef __DP_MATRIX_INTEGER_H__
#define __DP_MATRIX_INTEGER_H__

#include <xasmlib/IntegerVector.h>

#include "bspcpp/tools/aligned_allocator.h"

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
	template <class input_t, class operator_t>
	class dp_matrix_integer_solver {
	public:
		dp_matrix_integer_solver() {
			left_dp_column = NULL;
			top_dp_row = NULL;
			tmp_dp = NULL;
			tmp_size = 0;
			left_col_size = 0;
			top_row_size = 0;
			cur_top_row = top_dp_row;
			cur_left_col = left_dp_column;
			prev_top_row = cur_top_row;
			prev_left_col = cur_left_col;
		}
		
		~dp_matrix_integer_solver() {
			if(left_dp_column != NULL) {
				alloc.deallocate(left_dp_column, 0);
			}
			if(top_dp_row != NULL) {
				alloc.deallocate(top_dp_row, 0);
			}
			if(tmp_dp != NULL) {
				alloc.deallocate(tmp_dp, 0);
			}
		}

		int * cur_top_row;
		int * prev_top_row;
		int * cur_left_col;
		int * prev_left_col;

	private:
		/*!
		* The row and column containing the initial values of the current block
		*/
		int * left_dp_column;
		int * top_dp_row;
		int * tmp_dp;

		size_t left_col_size;
		size_t top_row_size;
		size_t tmp_size;

		/*!
		* the operator. 
		*/
		operator_t op;

		utilities::aligned_allocator<int> alloc;

	public:
		void init_left(size_t size, int value) {
			if(left_col_size < size) {
				if(left_dp_column != NULL)
					 alloc.deallocate(left_dp_column, 0);
				left_dp_column = alloc.allocate(size);
				left_col_size = size;
				memset(left_dp_column, value, sizeof(int)*size);
			}
		}
		
		void init_top(size_t size, int value) {
			if(top_row_size < size) {
				if(top_dp_row != NULL)
					 alloc.deallocate(top_dp_row, 0);
				top_dp_row = alloc.allocate(size);
				top_row_size = size;
				memset(top_dp_row, value, sizeof(int)*size);
			}
		}

		void init_tmp(size_t size, int value) {
			if(tmp_size < size) {
				if(tmp_dp != NULL)
					 alloc.deallocate(tmp_dp, 0);
				tmp_dp = alloc.allocate(size);
				tmp_size = size;
				memset(tmp_dp, value, sizeof(int)*size);
			}
		}
		
		UINT64 operator() (input_t left_input, input_t top_input) {
			size_t m = left_input.size();
			size_t n = top_input.size();
			int* tmp;

			init_left(m+1, 0);
			init_top(n+1, 0);
			init_tmp(n+1, 0);

			cur_left_col = left_dp_column;
			prev_top_row = top_dp_row;
			cur_top_row = tmp_dp;

			for (register size_t i = 1; i <= m; ++i) {
				cur_top_row[0]= cur_left_col[i];
				for (register size_t j = 1; j <= n; ++j) {
					cur_top_row[j] = op(i, j, left_input[i-1], top_input[j-1], cur_top_row[j-1], prev_top_row[j-1], prev_top_row[j]);
				}
				tmp = cur_top_row;
				cur_top_row = prev_top_row;
				prev_top_row = tmp;
//				swap(cur_top_row, prev_top_row);
				cur_left_col[i] = cur_top_row[n];
			}
			return cur_left_col[m];
		}
	};
};

#endif

