/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef SCOREMATRIX_H_
#define SCOREMATRIX_H_

#include "defs.h"
#include "rangesearching/Range2D.h"

#include "lcs/Llcs.h"

namespace seaweeds {

/**
 * @brief score matrix class.
 * 
 */
template< class _storage > // = ExplicitStorage<lcs::Llcs<std::string, int>, int> >
class ScoreMatrix : public _storage {
    public: 
		typedef ScoreMatrix<_storage> _my_type;
		typedef typename _storage::string string;
		typedef typename _storage::archive archive;

    	ScoreMatrix(int _m, int _n) : 
			_storage(_m, _n) {}

    	ScoreMatrix(int _m, int _n, archive _a) :
			_storage(_m, _n, _a) {}

		/**
		 * \brief returns the score corresponding to pair (i, j)
		 * \param i 
		 * \param j 
		 * \return 
		 */
		int score(int i, int j) {
			return j - i - _storage::distribution(i, j);
		}

        static void dumpdistribution(std::ostream & stream, 
        		  ScoreMatrix & m) {
        	stream << "distribution (" << m.m << "," << m.n << ") = " << std::endl;
    		
        	for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "" << j << "\t";
    		}
        	stream << std::endl;
        	for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "--------";
    		}
        	stream << std::endl;
        	stream << std::endl;
        	
        	for(int i = -m.m-1; i < m.n+1; ++i) {
        		for(int j = -1; j < m.m+m.n+1; ++j) {
        			stream << m.distribution(i, j) << ",\t";
        		}
				stream << "| "<< i  << std::endl;
        	}
    		for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "--------";
			}
    		stream << std::endl;
        }

        static void dumpscore(std::ostream & stream, 
        		  ScoreMatrix & m) {
        	stream << "scores (" << m.m << "," << m.n << ") = " << std::endl;
    		
        	for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "" << j << "\t";
    		}
        	stream << std::endl;
        	for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "--------";
    		}
        	stream << std::endl;
        	stream << std::endl;
        	
        	for(int i = -m.m-1; i < m.n+1; ++i) {
        		for(int j = -1; j < m.m+m.n+1; ++j) {
        			stream << m.score(i, j) << ",\t";
        		}
				stream << "| "<< i  << std::endl;
        	}
    		for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "--------";
			}
    		stream << std::endl;
        }

        static void dumpdensity(std::ostream & stream, 
        		  ScoreMatrix & m) {
        	stream << "density (" << m.m << "," << m.n << ") = " << std::endl;
    		
        	for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "" << j << "\t";
    		}
        	stream << std::endl;
        	for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "--------";
    		}
        	stream << std::endl;
        	stream << std::endl;
        	
        	for(int i = -m.m; i < m.n+2; ++i) {
        		for(int j = -1; j < m.m+m.n+1; ++j) {
        			stream << m.density(i, j) << ",\t";
        		}
				stream << "| "<< i  << std::endl;
        	}
    		for(int j = -1; j < m.m+m.n+1; ++j) {
    			stream << "--------";
			}
    		stream << std::endl;
        }

};

};

#include "sm_ExplicitStorage.h"
#include "sm_ImplicitStorage.h"

#endif /*SCOREMATRIX_H_*/


