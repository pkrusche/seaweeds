/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef __NWALIGNMENT_H__
#define __NWALIGNMENT_H__

#include <string>
#include <algorithm>

#include <sstream>

#include "dynamicprogramming/DynamicProgrammingMatrixSolver.h"
#include "bsp_cpp/bsp_cpp.h"
#include "util/global_options.h"

namespace alignment {

template <class _string = utilities::IntegerVector<8> > 
class PairwiseScoringOperator {
private:
	int * * subst_matrix;
	
	double normalization;

	int gap_score_h;
	int gap_score_v;
    
    int gap_continuation_score_h;
    int gap_continuation_score_v;
    
	bool lcase_characters;
	std::string allowed_characters; 
	size_t alphasize;

	char translation_map_1[256];
	char translation_map_2[256];

public:
	PairwiseScoringOperator() {
		using namespace std;
		using namespace bsp;
		string _subst_matrix;
		string allowed_characters;
		int _lcase_characters;
		string _translation_map_1;
		string _translation_map_2;

/*		Human mouse: 
 		91		-114		-31			-123
		-114	 100		-125		-31
		-31		-125		100			-114
		-123	-31			-114		91

		gap_score_h = -50;		
		gap_score_v = -50;

		normalization = 0.01; */

		global_options.get("alignment::PairwiseScoringOperator::allowed_characters", allowed_characters, "acgtn");
		global_options.get("alignment::PairwiseScoringOperator::translation_map_1", _translation_map_1, "0 1 2 3 4");
		global_options.get("alignment::PairwiseScoringOperator::translation_map_2", _translation_map_2, "0 1 2 3 5");

		global_options.get("alignment::PairwiseScoringOperator::lcase_characters", _lcase_characters, 1);
		lcase_characters = _lcase_characters != 0;

		global_options.get("alignment::PairwiseScoringOperator::subst_matrix", 
							_subst_matrix,
/*       A C G T N N' */
/* A */ "2 0 0 0 0 0 "
/* C */ "0 2 0 0 0 0 "
/* G */ "0 0 2 0 0 0 "
/* T */ "0 0 0 2 0 0 "
/* N */ "0 0 0 0 0 0 "
/* N'*/ "0 0 0 0 0 0 "
);
		global_options.get("alignment::PairwiseScoringOperator::gap_score_h", gap_score_h, -1);
		global_options.get("alignment::PairwiseScoringOperator::gap_score_v", gap_score_v, -1);
		global_options.get("alignment::PairwiseScoringOperator::gap_continuation_score_h", gap_continuation_score_h, -1);
		global_options.get("alignment::PairwiseScoringOperator::gap_continuation_score_v", gap_continuation_score_v, -1);
		global_options.get("alignment::PairwiseScoringOperator::normalization", normalization, 0.5);

		// construct alphabet translation maps.
		memset(translation_map_1, 0, 256);
		memset(translation_map_2, 0, 256);
		
		istringstream tmis1(_translation_map_1);
		char kmax = 0;
		int pos = 0;
		while(!tmis1.eof() && pos < allowed_characters.size() ) {
			int k;
			tmis1 >> k;
			translation_map_1[allowed_characters.at(pos)] = (char) k;
			kmax = max((char)k, kmax);
			pos++;
		}

		istringstream tmis2(_translation_map_2);
		pos = 0;
		while(!tmis2.eof() && pos < allowed_characters.size() ) {
			int k;
			tmis2 >> k;
			translation_map_2[allowed_characters.at(pos)] = (char) k;
			kmax = max((char)k, kmax);
			pos++;
		}

		alphasize = ((size_t)kmax)+1;

		// replace separator characters with whitespace
		subst_matrix = new int* [alphasize];
		for(size_t s=0; s < _subst_matrix.size(); ++s) {
			if(_subst_matrix[s] == '['
			|| _subst_matrix[s] == ']'
			|| _subst_matrix[s] == ','
			|| _subst_matrix[s] == ';'
			|| _subst_matrix[s] == '\n'
			) {
				_subst_matrix[s] = ' ';
			}
		}

		istringstream is(_subst_matrix);
		for (int i = 0; i < alphasize; ++i)	{
			subst_matrix[i] = new int [alphasize];
		}

		for (int j = 0; j < alphasize; ++j)	{            
			for (int i = 0; i < alphasize; ++i)	{
				is >> subst_matrix[i][j];
			}
		}
/*        static int jj = 0;
        cout << "subst matrix: [ " << endl;
		// read row by row.
		for (int i = 0; i < alphasize; ++i)	{
            for (int j = 0; j < alphasize; ++j)	{            
                cout << subst_matrix[i][j] << " ";
			}
            cout << ";" << endl;
		}
        cout << "] " << jj++ << endl; */
	}

	_string translate_input(std:: string const & str, bool first = true) {
		using namespace std;
		_string translated (str.length());
		char * charmap = first ? translation_map_1 : translation_map_2;

		for (size_t i = 0; i < translated.size(); ++i) {
			char current = str[i];
			if(lcase_characters) {
				current = tolower(current);
			}
			translated[i] = charmap[current];
		}

		return translated;
	}

	typedef double score_t ;
	typedef int dp_element_t;
    
	inline int operator() (size_t i, size_t j,
		char top, 
		char left,
		int tleft,
		int ttop_left,
		int ttop
		) {
		using namespace std;

        // MSB gives gap
        bool gap_left = (tleft & 1) > 0;
        bool gap_top  = (ttop  & 2) > 0;
        
        tleft     >>= 2;
        ttop      >>= 2;
        ttop_left >>= 2;
        
        int score_l = gap_left ? tleft + gap_continuation_score_h : tleft + gap_score_h;
        int score_t = gap_top  ? ttop  + gap_continuation_score_v : ttop  + gap_score_v;
        int score_tl = ttop_left + subst_matrix[left][top];
        
        int score_ret = 0;

        if (score_l > score_tl) {
            if (score_l > score_t) {
            	score_ret = score_l << 2 | 1;
            } else if (score_l == score_t) {
            	score_ret = score_l << 2 | 3;
            } else {
            	score_ret = score_t << 2 | 2;            	
            }
        } else {
        	score_ret = score_tl << 2;
        }
        
		return score_ret;
	}

	inline int init_l() {
		return 0;
	}

	inline int init_t() {
		return 0;
	}

	double convert_score (int score, size_t m, size_t n) {
		return normalization * ((double)(score >> 2));
	}

};
	
template <class _string = std::string, class _op = PairwiseScoringOperator < > >
class NWAlignment : public dynamic_programming::DynamicProgrammingMatrixSolver<_string, typename _op::score_t, _op> {
public:
	typedef  typename _op :: score_t score_t;
	typedef  typename _op :: dp_element_t dp_element_t;

	_op scoring;

	score_t operator() (_string left_input, _string top_input) {
		return scoring.convert_score(
			((dynamic_programming::
				DynamicProgrammingMatrixSolver<_string, typename _op :: score_t, _op>)*this)(left_input, top_input),
			left_input.size(),
			top_input.size()
		);
	}
};

};

#endif

