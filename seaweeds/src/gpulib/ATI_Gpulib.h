/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef ATI_Gpulib_h__
#define ATI_Gpulib_h__

#include "brook/Stream.h" 

class GPUSeaweeds {
public:
	GPUSeaweeds();
	~GPUSeaweeds();

	void run(const char * x, const char * y, unsigned int m, unsigned int n, int * permutation);
	void finish(int * permutation);
private:
	int m, n;
	brook::Stream<char> * x_stream;
	brook::Stream<char> * y_stream;
	brook::Stream<int> * p_stream[2];
	int _in;
	int _out;
	};



#endif // ATI_Gpulib_h__

