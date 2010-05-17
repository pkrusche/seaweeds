/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include "seaweeds_gpu.h"

#include <iostream>
#include <algorithm>

#include "ATI_Gpulib.h"


GPUSeaweeds::GPUSeaweeds() : x_stream(NULL), y_stream(NULL) {
	p_stream[0] = NULL;
	p_stream[1] = NULL;
}

GPUSeaweeds::~GPUSeaweeds() {
	if(x_stream) {
		delete x_stream;
	}
	if(y_stream) {
		delete y_stream;
	}
	if(p_stream[0]) {
		delete p_stream[0];
	}
	if(p_stream[1]) {
		delete p_stream[1];
	}
}

void GPUSeaweeds::run(const char * x, const char * y, unsigned int m, unsigned int n, int * permutation) {
	using namespace std;
	using namespace brook;

	unsigned int plen = m+n;

	// if the given container is of the correct size, we take the 
	// values in it as inputs
	x_stream = new Stream<char>(1, &m);
	y_stream = new Stream<char>(1, &n);

	p_stream[0] = new Stream<int>(1, &plen);
	p_stream[1] = new Stream<int>(1, &plen);

	x_stream->read(x);
	y_stream->read(y);
	p_stream[0]->read(permutation);

	_in = 0;
	_out = 1;

	unsigned int j = 0;
	int mid = m;
	int h = 1;
	int x_start = 0;
	int y_end   = 0;

	for (j = 1; j < m+n; ++j) {
		int offset = mid-h;
		int len = 2*h;

		compnet_stage(
			*x_stream, 
			*y_stream, 
			offset,
			len,
			x_start, y_end,
			*p_stream[_in],
			*p_stream[_out]
		);
		swap(_in, _out);

		if (j < m && j < n) { // first triangle
			h+= 1;
			y_end+= 1;
		} else if(j < m && j >= n) { // m > n -> need to move middle down. have h = n now
			mid-= 1;
			x_start+= 1;
		} else if(j < n && j >= m) { // m < n -> middle moves up. h = m
			mid+= 1;
			y_end+= 1;
		} else { // j >= m && j >= n
			h-= 1;
			x_start+= 1;
		}

		// DEBUGability
		/*
		p_stream[_in]->write(permutation);
		for (int k = 0; k < m+n; ++k) {
		cout << std::setw(2) << permutation[k] << " ";
		}		
		cout << endl;
		*/
	}
	swap(_in, _out);
	invert_permutation(*p_stream[_in], *p_stream[_out]);
}

void GPUSeaweeds::finish(int * permutation) {
	p_stream[_out]->write(permutation);
	if(x_stream) {
		delete x_stream;
		x_stream = NULL;
	}
	if(y_stream) {
		delete y_stream;
		y_stream = NULL;
	}
	if(p_stream[0]) {
		delete p_stream[0];
		p_stream[0] = NULL;
	}
	if(p_stream[1]) {
		delete p_stream[1];
		p_stream[1] = NULL;
	}
}
