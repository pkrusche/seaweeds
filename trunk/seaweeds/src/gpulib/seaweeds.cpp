/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef seaweeds_cpp__
#define seaweeds_cpp__

#include "seaweeds_gpu.h"

#include <iostream>
#include <algorithm>

#include "ATI_Gpulib.h"

namespace seaweeds {

MultiSeaweeds_GPU::MultiSeaweeds_GPU(int _y_chunksize) : y_chunksize(_y_chunksize) {
	using namespace std;
}

MultiSeaweeds_GPU::~MultiSeaweeds_GPU() {
}

void MultiSeaweeds_GPU::run(const string * x, unsigned int k, const string & y) {
	using namespace std;
	using namespace brook;
	
	brook::Stream<char> * x_stream;	///< string storage on the GPU for the first inputs
	brook::Stream<char> * y_stream;	///< string storage on the GPU for the first inputs
	brook::Stream<int> * p_stream_1; ///< temporary output buffering streams
	brook::Stream<int> * p_stream_2; ///< temporary output buffering streams

	int n_remaining = y.size();
	m = x[0].size();
	int * permutation_temp = alloc.allocate((m+n_remaining) * k);
	
	// create k identity input permutations
	for (int _k = 0; _k < k; ++_k) {
		for (int j = 0; j < n_remaining+m; ++j) {
			permutation_temp[_k + j*k] = j;
		}
	}
	
	unsigned int xdim[] = { m, k };
	char * xdata = (char*)alloc.aligned_malloc(xdim[0]*xdim[1]);
	x_stream = new Stream<char>(2, xdim);
	for (int _k = 0; _k < k; ++_k) {
		for (int j = 0; j < xdim[0]; ++j) {
			xdata[m*_k + j] = x[_k][j];
		}
	}
	x_stream->read(xdata);
	
	// allocate the permutation streams
	unsigned int pdim[] = {k, m+y_chunksize};
	p_stream_1 = new Stream<int>(2, pdim);
	p_stream_2 = new Stream<int>(2, pdim);
	// this stream stores the current chunk of y in GPU memory
	y_stream = new Stream<char>(1, &y_chunksize);
	
	int p_pos = 0;
	int y_pos = 0;
	while (n_remaining > 0) {
		n = min(y_chunksize, (unsigned)n_remaining);
		n_remaining-= n;

		// last chunk might be smaller
		if (n < y_chunksize) {
			delete p_stream_1;
			delete p_stream_2;
			delete y_stream;
			pdim[1] = n+m;
			p_stream_1 = new Stream<int>(2, pdim);
			p_stream_2 = new Stream<int>(2, pdim);
			y_stream = new Stream<char>(1, &n);
		}
		brook::Stream<int> * p_stream[] = {p_stream_1, p_stream_2}; ///< temporary output buffering streams
	
/*
		cout << "Starting chunk" << endl;
		for (int _k = 0; _k < k; ++_k) {
			cout << _k << ": ";
			for (int l = 0; l < pdim[1]; ++l) {
				cout << permutation_temp[p_pos + _k + l*k] << "\t";
			}
			cout << endl;
		}
*/
		p_stream_1->read(permutation_temp + p_pos);
		y_stream->read(((char *)y.data()) + y_pos);

		int _in = 0;
		int _out = 1;

		int mid = m;
		int h = 1;
		int x_start = 0;
		int y_end   = 0;

		for (unsigned int j = 1; j < m+n; ++j) {
			int offset = mid-h;
			int len = 2*h;	

			compnet_stage_2d(
				*x_stream, 
				*y_stream, 
				offset, len, x_start, y_end, 
				*(p_stream[_in]), 
				*(p_stream[_out])
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
		}
		p_stream[_in]->write(permutation_temp+p_pos);
/*
		for (int _k = 0; _k < k; ++_k) {
			cout << _k << ": ";
			for (int l = 0; l < pdim[1]; ++l) {
				cout << permutation_temp[p_pos + _k + l*k] << "\t";
			}
			cout << endl;
		}

*/
		p_pos+= n*k;
		y_pos+= n;
	}

	vector<int> v;
	p_outputs.resize(k, v);
	n = y.size();
	int pos = 0;
	for (int _k = 0; _k < k; ++_k) {
		p_outputs[_k].resize(m+n, pos);
		for (int j = 0; j < m+n; ++j) {
			p_outputs[_k][permutation_temp[_k+j*k]] = j;
		}
	}
	alloc.deallocate(permutation_temp, 0);

	if(x_stream->error()) {
		cerr << "x_stream error: " <<  x_stream->errorLog();
	}
	if(y_stream->error()) {
		cerr << "y_stream error: " << y_stream->errorLog();
	}
	if(p_stream_1->error()) {
		cerr << "p_stream_1 error: "<< p_stream_1->errorLog();
	}
	if(x_stream->error()) {
		cerr << "p_stream_2 error: " << p_stream_1->errorLog();
	}

	delete x_stream;
	delete y_stream;
	delete p_stream_1;
	delete p_stream_2;
}

std::vector<int> & MultiSeaweeds_GPU::get_seaweedpermutation(int k) {
	return p_outputs[k];
}

};

#endif // seaweeds_cpp__

