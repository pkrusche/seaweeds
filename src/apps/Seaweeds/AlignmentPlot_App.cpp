/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include <boost/shared_array.hpp>
#include <fstream>

#include "bsp_cpp/bsp_cpp.h"
#include "AlignmentPlot_App.h"

#include "Methods/AlignmentPlot_Method.h"
#include "AlignmentPlotIO.h"

#include <tbb/mutex.h>

tbb::mutex ap_output_mutex;

class AlignmentPlot_Parallel : public bsp::Context {
public:

	AlignmentPlot_Parallel() {
		CONTEXT_SHARED_INIT(sequence_1, std::string);
		CONTEXT_SHARED_INIT(sequence_2, std::string);
		CONTEXT_SHARED_INIT(method, std::string);
		CONTEXT_SHARED_INIT(windowlength, int);
		CONTEXT_SHARED_OBJECT(ap, AlignmentPlot);
	}

	void set_parameters(
		std::string _sequence_1,
		std::string _sequence_2,
		std::string _method,
		int _windowlength
	) {
		sequence_1 = _sequence_1;
		sequence_2 = _sequence_2;
		method = _method;
		windowlength = _windowlength;

		ap.set_parameters(
			(int)sequence_1.length(), 
			(int)sequence_2.length(),
			windowlength, 
			0, 
			windowlength // max score = all matches
			);
	}
	void run() {
		using namespace std;
		BSP_SCOPE(AlignmentPlot_Parallel);

		double t0 = bsp_time();

		BSP_BEGIN();
		using namespace std;

		ap.set_parameters(
			(int)sequence_1.length(), 
			(int)sequence_2.length(),
			windowlength, 
			0, 
			windowlength+0.1 // max score = all matches
			);


		AlignmentPlot_Method_Ptr apm = 
			AlignmentPlot_Method::get_method(method, ap);

		// number of windowlength * length(seq2) strips
		int strips = (int)sequence_1.length() - windowlength + 1;
		int strips_per_proc = ICD(strips, bsp_nprocs());

		int my_x0 = bsp_pid()*strips_per_proc;
		int my_x1 = (bsp_pid()+1)*strips_per_proc;

		if (my_x1 >= strips)
		{
			my_x1 = strips-1;
		}

		if (my_x1 < my_x0) {
			return;
		}

		int my_len = my_x1-my_x0 + 1;

		{
			tbb::mutex::scoped_lock l (ap_output_mutex);
			cout << "p" << bsp_pid() << ": strips=" << strips 
				<< " x0=" << my_x0 << " my_len= " << my_len << endl;
		}

		ASSERT(my_len >= 1);

		apm->run(sequence_1.substr(my_x0, my_len+windowlength-1), sequence_2, my_x0, 0);

#ifdef _DEBUG
				std::cout << "\n" ;
#endif

		BSP_END();

		double t1 = bsp_time();
		std::cout << "Alignment time: " << (t1-t0) << std::endl;
	}

	void dump(std::string const & name) {
		AlignmentPlotIO io;
		io.init_from(ap);
		io.write_text(name.c_str());

		{
			std::ofstream jsonout((name + ".json").c_str());
			jsonout << io;
		}
	}

protected:
	std::string sequence_1;
	std::string sequence_2;
	std::string method;

	int windowlength;

	AlignmentPlot ap;
};


void AlignmentPlot_App::run( boost::program_options::variables_map & vm ) {
	using namespace std;
	int    processors = vm["processors"].as<int>();
	
	string first_file  = vm["first-sequence"].as<string>();
	string second_file = vm["second-sequence"].as<string>();
	string output      = vm["output-file"].as<string>();

	int windowlength   = vm["windowsize"].as<int>();
	string method     = vm["method"].as<string>();

	/* pid == 0? read input! */
	string seq1, seq2;
	if(bsp_pid() == 0) {
		std::ifstream ff(first_file.c_str());
		seq1 = TextIO::read_multiline_string(ff);
		std::ifstream sf(second_file.c_str());
		seq2 = TextIO::read_multiline_string(sf);
		seq1 = TextIO::trim(seq1);
		seq2 = TextIO::trim(seq2);
	}


	/* and distribute */ 
	bsp::bsp_broadcast(0, seq1);
	bsp::bsp_broadcast(0, seq2);

	if (seq1.length() < windowlength || seq2.length() < windowlength ) {
		bsp_abort ("One or more of the inputs is too short. l1 = %i, l2 = %i", 
			seq1.length(), seq2.length());
	}

	/** cap number of processors for very short sequences */
	if(processors > ((int)seq1.length() - windowlength + 1)) {
		processors = (int)seq1.length() - windowlength + 1;
	}

	cout << "Processors: " << processors << " S1: " 
		<< seq1.length() << " S2: " << seq2.length() << " W: " << windowlength << endl;

	bsp::Runner<AlignmentPlot_Parallel> al_runner (processors);
	al_runner.set_parameters(seq1, seq2, method, windowlength);
	al_runner.run();

	if(bsp_pid() == 0) {
		cout << "Writing output: " << output << endl;
		al_runner.dump(output);
	}
}

