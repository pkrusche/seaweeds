/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "PSSM_Tool.h"

/************************************************************************/
/* PSSM Pairwise distance App                                           */
/************************************************************************/

/**
 * Pairwise distance matrix class
 */
class Distances {
private:
	int N;
	utilities::AVector<double> distances;

public:
	Distances () {
		N = 0;
	}

	inline void resize (int _n) {
		N = _n;
		distances.resize( N * N );
		for (int j = 0; j < N * N; ++j) {
			distances [j] = -1.0;
		}
	}

	inline double & operator() (int i, int j) {
		return distances[i*N + j];
	}

	inline void normalize() {
		using namespace std;
		double mi = DBL_MAX, ma = 0;
		for (int j = 0; j < N * N; ++j) {
			mi = min (mi, distances[j]);
			ma = max (ma, distances[j]);
		}
		for (int j = 0; j < N * N; ++j) {
			distances[j] = (distances[j] - mi) / (ma - mi);
		}
	}

	inline void * data () {
		return distances.data;
	}

	inline size_t data_size() {
		return N*N * sizeof(double);
	}
};

/** Parallel code for computing pairwise distances 
 * 
 * Uses g_pssms.
 * 
 */
class PSSM_Distances : public bsp::Context {
public:
	enum distance_t {
		DIST_KULLBACK_LEIBLER,
		DIST_HELLINGER,
	};

	PSSM_Distances() {
		CONTEXT_SHARED_INIT(dt, distance_t);

		dt = DIST_KULLBACK_LEIBLER;
		N = (int) g_pssms.size();
		data = new double [N*N];
		i_have_the_result = false;
	}

	~PSSM_Distances() {
		delete [] data;
	}

	/** Set the distance type, default: Kullback-Leibler */
	void set_distance_type (distance_t _dt) {
		dt = _dt;
	}

	static bool result_ready() {
		return i_have_the_result;
	}

	static Distances & get_distances () {
		ASSERT(i_have_the_result);
		return dists;
	}

protected:

	static double KD ( pssms::PSSM<4> const & a , pssms::PSSM<4> const & b ) {
		static pssms::Both_Strands_DNA_Min_KullbackLeibler_Distance d;
		return d(a, b);
	}

	static double HD ( pssms::PSSM<4> const & a , pssms::PSSM<4> const & b ) {
		static pssms::Both_Strands_DNA_Min_Hellinger_Distance d;
		return d(a, b);
	}

	boost::function2<double, pssms::PSSM<4> const &, pssms::PSSM<4> const & > distancefun;

	/** Context variables */
	int P, N, p, n, my_start, my_end;
	double * data;
	distance_t dt;

	/** Result output */
	static bool i_have_the_result;
	static Distances dists;

	void run () {
		using namespace std;
		BSP_SCOPE(PSSM_Distances);
		BSP_BEGIN();
		
		switch (dt) {
		case DIST_HELLINGER:
			distancefun = &HD;
			break;
		default:
			distancefun = &KD;
			break;
		}


		P = bsp_nprocs();
		p = bsp_pid();

		n = ICD(N, P);

		my_start = n * p;
		my_end = n * (p+1) - 1;


		if (my_end >= N) {
			my_end = N-1;
		}

		bsp_push_reg(data, N*N*sizeof(double));

		BSP_SYNC();

		int l = 0;
		for (int j = my_start; j <= my_end; ++j) {
			for (int k = 0; k < N; ++k) {
				if (j == k) {
					data[l*N+k] = 0;
				} else {
					data[l*N+k] = distancefun(g_pssms[j], g_pssms[k]);
				}
				add_progress(1);
			}
			++l;
		}

		if (bsp_pid() != 0 && my_start <= my_end) {
			bsp_hpput(0, data, data, my_start * N *sizeof(double), (my_end - my_start + 1) * N * sizeof(double));
		}

		BSP_SYNC();
		bsp_pop_reg(data);

		if ( bsp_pid() == 0 ) {
			i_have_the_result = true;
			dists.resize(N);
			memcpy(dists.data(), data, N * N * sizeof(double));
		}

		BSP_END();
	}

};

/** static implementation */
bool PSSM_Distances::i_have_the_result;
Distances PSSM_Distances::dists;

 /** run the app and output results on one node */
 void PSSM_DistancesApp::run (boost::program_options::variables_map & vm) {
	using namespace std;
	int distancefun = vm["distancetype"].as<int>();

	if (distancefun < 0 || distancefun > 1) {
		throw std::runtime_error( "Unknown distance function");
	}

	bsp::Runner<PSSM_Distances> r;
	if (distancefun == 0) {
		r.set_distance_type(PSSM_Distances::DIST_HELLINGER);
	} else {
		r.set_distance_type(PSSM_Distances::DIST_KULLBACK_LEIBLER);
	}
	start_progress("Computing pairwise distances...", (int) ( g_pssms.size() * g_pssms.size() ) );
	r.run();
	end_progress();

	/** only happens on one node */
	if (r.result_ready()) {
		Distances & d(r.get_distances());

		if (vm.count("normalize") > 0) {
			d.normalize();
		}

		for (size_t i = 0; i < g_pssms.size(); ++i) {
			cout << g_pssms[i].get_name() << "\t" ;
		}
		cout << "X" << endl;
		for (size_t i = 0; i < g_pssms.size(); ++i) {
			cout << g_pssms[i].get_name() << "\t";
			for (size_t j = 0; j < g_pssms.size(); ++j) {
				double distance = d ((int)i, (int)j);
				cout << distance << "\t";
			}
			cout << endl;
		}
	}
}

