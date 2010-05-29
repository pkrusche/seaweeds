/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#include "bspcpp/bsp_cpp.h"
#include <tbb/mutex.h>
#include <loki/Typelist.h>
#include <iostream>

tbb::mutex g_output_mutex;

struct Context {
	
};

BSP_SUPERSTEP_DEF_BEGIN(Superstep1, int, bsp::ProcMapper)
	tbb::mutex::scoped_lock l;
	l.acquire(g_output_mutex);
	std::cout << "Hello from process " << bsp_pid() << " out of " << bsp_nprocs() 
		<< ". I live on node " << ::bsp_pid() << ", where i am local pid " << bsp_local_pid() << "." << std::endl
		<< "Also, I have value " << context << ", executing Superstep1" << std::endl;
	context++;
	l.release();
BSP_SUPERSTEP_DEF_END()


BSP_SUPERSTEP_DEF_BEGIN(Superstep2, int, bsp::ProcMapper)
	tbb::mutex::scoped_lock l;
	l.acquire(g_output_mutex);
	std::cout << "Hello from process " << bsp_pid() << " out of " << bsp_nprocs() 
		<< ". I live on node " << ::bsp_pid() 
			<< ", where i am local pid " << bsp_local_pid() << "." << std::endl
		<< "Also, I have value " << context << ", executing Superstep2" << std::endl;
	context--;
	l.release();
BSP_SUPERSTEP_DEF_END()

typedef bsp::FlatComputation<LOKI_TYPELIST_4(Superstep1, Superstep1, Superstep2, Superstep2), int> MyFlatParallelComputation;

void runner(void) {
	bsp_begin(-1);
	
	MyFlatParallelComputation::procmapper_t mapper(8);
	MyFlatParallelComputation::run(mapper);

	bsp_end();
}

int main(int argc, char ** argv) {
	bsp_init(runner, argc, argv);
	runner();
	return 0;
}
