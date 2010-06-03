/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

//#define _VERBOSETEST_WINDOWLCS
//#define  _SEAWEEDS_VERIFY
//#define  _VERBOSETEST_WINDOWLCS_CIPR

#include "bspcpp/bsp_cpp.h"
#include "autoconfig.h"

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cmath>

#include <tbb/task_scheduler_init.h>
#include <tbb/mutex.h>

#include "windowwindow/alignmentplotcomputation.h"
#include "windowwindow/postprocesswindows.h"

// the number of bits per character in our input strings
#define BPC	8

// the number of bits for a cell in the seaweed matcher
// windows of size 2^(SEAWEED_BPC-1) - 3 are supported
#define SEAWEED_BPC 8

using namespace std;
using namespace utilities;
using namespace lcs;
using namespace windowlocal;
using namespace windowlcs;

/**
 * BSPonMPI forces us to make the program options global if we want to use 
 * them in the SPMD part.
 */
int		g_argc;
char ** g_argv;

/**
 * we use a mutex so the output doesn't get mixed up
 */
tbb::mutex g_cout_mutex;

/**
 * blocking parameters
 */
size_t g_a_size;
size_t g_block_size;
size_t g_block_size_per_proc;
size_t g_data_size_per_proc;
size_t * g_displs;
size_t g_profile_size_per_proc;

int g_windowlength;
int g_firststepwidth; 
int g_secondstepwidth; 

AlignmentPlotComputation<BPC, SEAWEED_BPC, string> g_apc;

bsp_global_handle_t g_a_string;
bsp_global_handle_t g_profile_a;
bsp_global_handle_t g_profile_b;

struct context {
	AlignmentPlotComputation<BPC, SEAWEED_BPC, string> my_apc;
	char * buffer;
	size_t data_offset;
	size_t data_size;
};

BSP_SUPERSTEP_DEF_BEGIN(GetSubstring, context, bsp::ProcMapper)
	context.data_offset = g_displs[bsp_pid()];
	context.data_size= g_data_size_per_proc;
	
	if(context.data_offset + context.data_size > g_a_size) {
		context.data_size = g_a_size - context.data_offset;
	}
	context.buffer = new char[context.data_size];
	bsp_global_hpget(g_a_string, context.data_offset, context.buffer, context.data_size);
BSP_SUPERSTEP_DEF_END()

BSP_SUPERSTEP_DEF_BEGIN(ComputeAlignments, context, bsp::ProcMapper)
	context.my_apc.get_a() = string(context.buffer, context.data_size);
	delete [] context.buffer;

	string resultfile;
	context.my_apc.parameters().get("outputidentifier", resultfile);
	int node = bsp_pid();
	stringstream str;
	str << resultfile << "_" << node;
	resultfile = str.str();

	{
		tbb::mutex::scoped_lock l(g_cout_mutex);
		cout << "proc " << node
			 << " starting at block " << node * g_block_size_per_proc 
			  << " (char " << context.data_offset << ")/" 
			 << g_block_size <<
			", will process " << (context.data_size-g_windowlength+1) / g_firststepwidth
			<< " blocks (" << context.data_size << " chars)" << endl;
		cout << flush;
	}

	context.my_apc.preprocess_inputs();
	context.my_apc.setoutput(resultfile, (int)context.data_offset);

	utilities::Checkpointable * computation = context.my_apc.generate_matcher();

	double comp_time_0 = utilities::time();
	computation->run();
	{
		tbb::mutex::scoped_lock l(g_cout_mutex);
		cout << "Computation time on proc " << node << " : " << utilities::time() - comp_time_0 << flush << endl;
	}
	delete computation;

	int firststepwidth;
	g_apc.parameters().get("firststepwidth", firststepwidth);

	// write a profile
	bsp_global_put(	context.my_apc.get_profile_a(), 
		g_profile_a, 
		sizeof(double) * bsp_pid() * g_profile_size_per_proc, 
		sizeof(double) * context.my_apc.get_profile_size_a());

	// write b profile
	bsp_global_put(	context.my_apc.get_profile_b(), 
		g_profile_b,
		sizeof(double) * context.my_apc.get_profile_size_b() * bsp_pid(), 
		sizeof(double) * context.my_apc.get_profile_size_b());
	
	// flush and close output
	context.my_apc.setoutput("");
BSP_SUPERSTEP_DEF_END()

typedef bsp::FlatComputation<
	TYPELIST_2(
		GetSubstring, 
		ComputeAlignments
	), context, bsp::ProcMapper<context> > parallel_apc;

void spmd_computation() {
	using namespace bsp;
	bsp_begin(0);
	double setup_time_0 = utilities::time();

	if(bsp_pid() == 0) {
		g_apc.parsecommandline(g_argc, g_argv);
		cout << "string lengths: a " << g_apc.get_a().size() << " b " << g_apc.get_b().size() << endl;		
		if (g_apc.get_a().size() == 0 || g_apc.get_b().size() == 0) {
			bsp_abort("ERROR: At least one input string is empty.");
		}
	}

	// broadcast the parameters
	bsp_broadcast(0, g_apc.parameters());
	// we use b on all procs.
	bsp_broadcast(0, g_apc.get_b());

	int num_threads = 1;

	g_apc.parameters().get("threads", num_threads, -1);
	if (num_threads <= 0 ) {
		num_threads = tbb::task_scheduler_init::default_num_threads();
	}
	
	// use nprocs*threads virtual processors
	int processors = bsp_nprocs() * num_threads;

	// computation on a is distributed. We move a to a distributed array.
	g_a_size = g_apc.get_a().size();
	bsp_broadcast(0, g_a_size);
	g_a_string = bsp_global_alloc(g_a_size);
	bsp_sync();
	if (bsp_pid() == 0) {
		bsp_global_hpput(g_apc.get_a().c_str(), g_a_string, 0, g_a_size);
	}

	// get window length and step size parameters
	g_apc.parameters().get("windowlength", g_windowlength);
	g_apc.parameters().get("firststepwidth", g_firststepwidth);
	g_apc.parameters().get("secondstepwidth", g_secondstepwidth);


	g_block_size = (g_a_size - g_windowlength + g_firststepwidth - 1) / g_firststepwidth;
	g_block_size_per_proc = (g_block_size + processors - 1 ) / processors;
	g_data_size_per_proc   = g_block_size_per_proc * g_firststepwidth + g_windowlength;
	g_profile_size_per_proc = ICD(g_data_size_per_proc - g_windowlength + 2, g_firststepwidth);

	// allocate global array for the profiles
	g_profile_a = bsp_global_alloc(sizeof(double) * g_profile_size_per_proc * processors);
	g_profile_b = bsp_global_alloc(sizeof(double) * g_apc.get_profile_size_b() * processors);
 
	bsp_sync();


	g_displs = new size_t [processors];

	for(int j = 0; j < processors; ++j) {
		g_displs[j] = j * g_block_size_per_proc * g_firststepwidth;
	}

	parallel_apc :: procmapper_t pm(processors);
	
	// initialize local inputs
	for (int j = 0; j < pm.procs_this_node(); ++j) {
		context & c = pm.get_context(j); 
		g_apc.assign_settings(c.my_apc);
		c.my_apc.get_b() = g_apc.get_b();
	}
	
	parallel_apc :: run (pm);

	delete [] g_displs;
	
	double pp_start = time();

	if (bsp_pid() == 0) {
		cout << "Postprocessing..." << endl;
		double * profile_a = new double[g_profile_size_per_proc*processors];
		double * profile_b = new double[g_apc.get_profile_size_b() * processors];
		
		bsp_global_get(g_profile_a, 0, profile_a, g_profile_size_per_proc * processors * sizeof(double));
		bsp_global_get(g_profile_b, 0, profile_b, g_apc.get_profile_size_b() * sizeof(double) * processors);
	
		bsp_sync();

		string profilefile1;
		string profilefile2;
		g_apc.parameters().get("profile1", profilefile1, "profile1.txt");
		g_apc.parameters().get("profile2", profilefile2, "profile2.txt");
		cout << "Profiles written to " << profilefile1 << " and " << profilefile2 << endl;
		ofstream f1(profilefile1.c_str(), ios::out);
		ofstream f2(profilefile2.c_str(), ios::out);
	
		for (size_t l = 0; l < g_apc.get_profile_size_a(); ++l) {
			size_t proc = min ((size_t)processors-1, l / g_block_size_per_proc);
			size_t offset = min (g_profile_size_per_proc, l - proc*g_block_size_per_proc);
			f1 << profile_a[proc * g_profile_size_per_proc + offset] << endl;
		}

		for (size_t l = 0; l < g_apc.get_profile_size_b(); ++l) {
			double val = profile_b[l];
			for (int p = 1; p < processors; ++p) {
				val = max(val, profile_b[l + p*g_apc.get_profile_size_b()]);
			}
			
			f2 << val << endl;
		}
		delete [] profile_a;
		delete [] profile_b;
	} else {
		bsp_sync();
	}

	bsp_sync();

	bsp_global_free(g_profile_a);
	bsp_global_free(g_profile_b);
	bsp_global_free(g_a_string);

	if(bsp_pid() == 0) {
		// sequential postprocessing

		string resultfile = g_apc.getoutputidentifier();
		const char ** args = new const char * [processors + 5];
		args[0] = "";
		for(int j = 1; j<= processors; ++j) {
			args[j] = new char [resultfile.size() + 100];
			sprintf((char*)args[j], "%s_%i", resultfile.c_str(), j-1);
		}
		args[processors+1] = "-o";
		args[processors+2] = (char*) resultfile.c_str();
		args[processors+3] = NULL;
		postprocess::postprocess(processors+3, args);
		for(int j = 1; j<= processors; ++j) {
			remove(args[j]);
			delete [] args[j];
		}
		delete [] args;
	}
	cout << "Postprocessing time: " << time() - pp_start << endl;

	bsp_end();
}

int main(int argc, char* argv[] ) {
	using namespace std; 
	utilities::init_xasmlib();
	g_argc = argc;
	g_argv = argv;
	bsp_init(spmd_computation, argc, argv);
	spmd_computation(); 
	return EXIT_SUCCESS;
}
