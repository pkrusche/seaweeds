/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "PSSM_Tool.h"

#include <vector>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/math/distributions/binomial.hpp>

#include <tbb/spin_mutex.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>

#include "datamodel/SequenceTranslation.h"

/************************************************************************/
/* Score PSSM matrices                                                  */
/************************************************************************/

struct Motif_Hit : public datamodel::Serializable {
	long int five_prime_pos;
	long int three_prime_pos;
	double pvalue;
	double score;
	std::string name;
	std::string accession;
	std::string sequence;
	std::string strand;

	int _pssm_idx;

	void defaults() {
		five_prime_pos = -1;
		three_prime_pos = -1;
		strand = "positive";
		sequence = "";
		name = "";
		pvalue = DBL_MAX;
		score = 0;
	}

	JSONIZE_AS (
		"Datatypes::Motifs::Motif_Hit",
		Motif_Hit, 1,
		S_STORE(name, JSONString<>)
		S_STORE(accession, JSONString<>)
		S_STORE(sequence, JSONString<>)
		S_STORE(strand, JSONString<>)
		S_STORE(five_prime_pos, JSONInt<long>)
		S_STORE(three_prime_pos, JSONInt<long>)
		S_STORE(pvalue, JSONDouble<>)
		S_STORE(score, JSONDouble<>)
	);
};

inline bool operator==(const Motif_Hit & m1, const Motif_Hit & m2) {
	return m1.name == m2.name &&
	( (m1.five_prime_pos == m2.five_prime_pos && m1.three_prime_pos == m2.three_prime_pos) ||
	  (m1.three_prime_pos == m2.five_prime_pos && m1.five_prime_pos == m2.three_prime_pos) );
}

struct Motif_Presence : public datamodel::Serializable {
	double pvalue;
	int count;
	int collected_scores;
	std::string test;
	std::string pssm_name;
	std::string pssm_accession;
	std::string sequence_name;

	void defaults() {
		sequence_name = "";
		pssm_name = "";
		pssm_accession = "";
		pvalue = DBL_MAX;
		test = "none";
	}

	JSONIZE_AS (
		"Datatypes::Motifs::Motif_Presence",
		Motif_Presence, 1,
		S_STORE(test, JSONString<>)
		S_STORE(sequence_name, JSONString<>)
		S_STORE(pssm_name, JSONString<>)
		S_STORE(pssm_accession, JSONString<>)
		S_STORE(pvalue, JSONDouble<>)
		S_STORE(count, JSONInt<>)
		S_STORE(collected_scores, JSONInt<>)
	);
};


static tbb::concurrent_queue<Motif_Hit> g_motif_queue;
/** the number of scores we collected for each PSSM. */
tbb::concurrent_vector<int> g_scores_collected;

/** Parallel PSSM Scorer */
class PSSM_Scorer : public bsp::Context {
public:
	PSSM_Scorer() {
		CONTEXT_SHARED_INIT(pos, int);
		CONTEXT_SHARED_INIT(current_sequence, dnastring);
	}

	static void set_parameters (boost::program_options::variables_map & vm) {
		scoretype = vm["scoretype"].as<std::string>();
		profiles = vm["profiles"].as<std::string>();

		pval = vm["pval"].as<double>();
		fragmentsize = vm["fragmentsize"].as<size_t>();

		overlap_size = 1;
		N = (int)g_pssms.size();
		minscores.resize(N);
		start_progress("Checking PSSMs...", N-1);
		profile_cache.resize( N );
		g_scores_collected.resize( N );
		for(int j = 0; j < N; ++j) {
			using namespace std;
			pssm & p = g_pssms[j];
			profile_cache[j] = new pssmhistogram;
			overlap_size = max (overlap_size, (size_t) p.get_length());
			add_progress(1);
			pssmhistogram * phi = profile_cache[j];
			PSSM_ProfileApp::get_profile (g_pssms[j], scoretype, *phi, profiles);
			minscores[j] = phi->right_tail_min_score(pval);
			g_scores_collected[j] = 0;
		}
		histograms_read = true;
		end_progress();
	}

	/** Set the sequence input */
	static void set_sequence_input(std::istream & _in) {
		in = &_in;
	}

	static void cleanup_profiles() {
		for (size_t j = 0; j < g_pssms.size(); ++j) {
			delete profile_cache[j];
		}
		minscores.clear();
		histograms_read = false;
	}

protected:

	/** Problem splitting (N/P pssms per processor) */
	int n, P, p;
	int  my_start, my_end;

	/** Current global sequence position */
	int pos;

	/** in each step, we read sequence into this and broadcast */
	dnastring current_sequence;

	/** Number of PSSMs */
	static int N;

	/** overlap size = maximum length of a pssm in g_pssms */
	static size_t overlap_size;

	/** Chunk size for reading the input */
	static size_t fragmentsize;

	/** the minimum p-value for a match */
	static double pval;

	/** initialized to the minimum scores necessary to score above a given p value for all g_pssms */
	static tbb::concurrent_vector<double> minscores;

	/** Input stream */
	static std::istream * in;

	/** the score type and profiles directory */
	static std::string scoretype, profiles;

	/** the profile cache */
	static tbb::concurrent_vector<pssmhistogram*> profile_cache;

	/** true when the histogram cache is valid */
	static bool histograms_read;

	void run() {
		BSP_SCOPE(PSSM_Scorer);

		start_progress("Reading Histograms...", N-1);

		BSP_BEGIN();
		P = bsp_nprocs();
		n = ICD(N, P);
		p = bsp_pid();

		my_start = p*n;
		my_end = (p+1)*n - 1;
		if (my_end >= N) {
			my_end = N-1;
		}

		BSP_END();
		end_progress();

		datamodel::TranslatingInputStream<8> stream (*in, "ACGTN");
		datamodel::WindowedInputStream<8> winstr ( overlap_size );
		winstr.set_input ( stream );

		pos = 0;
		for(int j = 0; j < N; ++j) {
			g_scores_collected[j] = 0;
		}
		current_sequence.resize( overlap_size );

		size_t read = 0;
		do {
			if ( ::bsp_pid() == 0 ) {
				read = winstr.get_window(fragmentsize, current_sequence, pos);
				current_sequence.resize(read);
				std::ostringstream s;
				s << "Read pos:" << pos << " (" << read << " chars)" << std::endl;
				if (read > 0) {
					start_progress(s.str().c_str(), N-1);
				}
			}

			BSP_BROADCAST(current_sequence, 0);
			BSP_BROADCAST(pos, 0);
			BSP_BEGIN();

			if (current_sequence.size() > 0) {
				for (int j = my_start; j <= my_end; ++j) {
					pssm & p = g_pssms[j];
					size_t len = p.get_length();
					double minscore = minscores[j];
					pssmscore scfun = pssmscorefactory::create(p, pssmscorefactory::s(scoretype.c_str()));

					for (int strand = 0; strand < 2; ++strand) {
						for (int k = 0; k < current_sequence.size() - len; ++k) {
							++g_scores_collected[j];
							double score = scfun->score(current_sequence, k);
							if (score > minscore) {
								pssmhistogram & px (*profile_cache[j]);
								dnastring testvec;
								current_sequence.extract_substring (k, k + len-1, testvec );

								long int fpp, tpp;
								std::string strnd;

								if (strand == 0) {
									strnd = "positive";
									fpp = pos+k;
									tpp = (long) ( pos+k+len-1 );
								} else {
									strnd = "negative";
									fpp = (long) (pos + (current_sequence.size() - 1 - k ));
									tpp = (long) (pos + (current_sequence.size() - 1 - (k+len-1) ) );
								}

								Motif_Hit hit;
								hit._pssm_idx = j;
								hit.name = p.get_name();
								hit.accession = p.get_accession();
								hit.five_prime_pos = fpp;
								hit.three_prime_pos = tpp;
								hit.strand = strnd;
								hit.score = score;
								hit.pvalue = px.right_tail(score);
								hit.sequence = datamodel::unwrap_sequence<8>(testvec, "ACGT", 'N');

								g_motif_queue.push (hit);
							}
						}
						datamodel::reverse_complement<8>(current_sequence, 4);
					}
					if ( ::bsp_pid() == 0 ) {
						add_progress(1);
					}
				}
			}

			BSP_END();
			if ( ::bsp_pid() == 0 && read > 0 ) {
				end_progress();
			}
		} while (read > 0);
	}
};

tbb::concurrent_vector<double> PSSM_Scorer::minscores;
tbb::concurrent_vector<pssmhistogram*> PSSM_Scorer::profile_cache;
std::istream * PSSM_Scorer::in = NULL;
std::string PSSM_Scorer::scoretype, PSSM_Scorer::profiles;
double PSSM_Scorer::pval;
int PSSM_Scorer::N;
size_t PSSM_Scorer::fragmentsize, PSSM_Scorer::overlap_size;
bool PSSM_Scorer::histograms_read = false;

struct _sorter {
	bool operator() (Motif_Hit const & i, Motif_Hit const & j) {
		return i.pvalue < j.pvalue;
	}
} sorter;

void PSSM_ScoreApp::run (boost::program_options::variables_map & vm) {
	using namespace std;
	using namespace boost::filesystem;

	string p = vm["sequence"].as<string>();
	double binom_pval = vm["binomial_p"].as<double>();
	int verbosity = vm["verbosity"].as<int>();
	vector<string> paths;
	TextIO::split(p, paths, ":");

	bsp::Runner<PSSM_Scorer> r;
	r.set_parameters(vm);

	for (size_t j = 0; j < paths.size(); ++j) {
		try {
			path inpath = paths[j];

			if ( is_directory(inpath) ) {
				for ( directory_iterator it(inpath), it_end; it != it_end; ++it) {
					const path & pr (it->path());
					std::string path_str = pr.string();

					if ( is_regular_file(pr) && path_str.find(".motifs.json") == string::npos ) {
						string s = it->path().string();
						paths.push_back(s);
						std::cerr << "Adding file " << s << endl;
					}
				}
				continue;
			}

			if (!exists(inpath) || !is_regular_file(inpath)) {
				throw std::runtime_error("Input sequence file was not found.");
			}

            std::ifstream f (inpath.c_str());

			r.set_sequence_input(f);
			r.run();

			std::vector< std::vector<Motif_Hit> > motifs;

			motifs.resize( g_pssms.size() );

			Motif_Hit hit;
			while (g_motif_queue.try_pop(hit)) {
				motifs[hit._pssm_idx].push_back(hit);
			}

            std::ofstream fout((inpath.string() + ".motifs.json").c_str());
			bool printed_one = false;
			fout << "[" << endl;

			for (size_t jj = 0; jj < g_pssms.size(); ++jj) {
				// no overlap allowed: we choose the ones
				// with the best p-values
				using namespace boost::icl;

				interval_map<int, Motif_Hit > hits;
				size_t hits_before = motifs[jj].size();

				for (size_t k = 0; k < hits_before; ++k) {
					Motif_Hit & hit (motifs[jj][k]);
					pair <discrete_interval<int>, Motif_Hit> interval;
					int start = min(hit.three_prime_pos, hit.five_prime_pos);
					int end = max(hit.three_prime_pos, hit.five_prime_pos);

					interval= make_pair(discrete_interval<int>::closed(start, end), hit);
					interval_map<int, Motif_Hit >::const_iterator it = hits.find ( interval.first );
					if (it == hits.end()) {
						hits.insert (interval);
					} else {
						if (it->second.pvalue > hit.pvalue) {
							hits.erase (it->first);
							hits.insert(interval);
						}
					}
				}

				size_t hitcount2 = 0;
				motifs[jj].clear();
				interval_map<int, Motif_Hit>::iterator it = hits.begin();
				while (it != hits.end()) {
					motifs[jj].push_back(it->second);
					++it;
					++hitcount2;
				}
				if (verbosity > 1) {
					cerr << "Hits: " << hits_before << ", after overlap removal: " << hitcount2 << endl;
				}

				// binomial test p-value filtering
				size_t motif_cutoff = 0;
				double min_pval = 1.0;
				if (binom_pval > 0) {
					sort (motifs[jj].begin(), motifs[jj].end(), sorter);
					if (verbosity > 2) {
						cerr << "binomial input pvals:";
						for (size_t k = 0; k < motifs[jj].size(); ++k) {
							cerr << " " << motifs[jj][k].pvalue;
						}
						cerr << endl;
						cerr << "binomial test: ";
					}
					for (size_t k = 0; k < motifs[jj].size(); ++k) {
						using namespace boost::math;
						double pval = cdf(complement(binomial(g_scores_collected[jj], motifs[jj][k].pvalue), k+1));
						if (verbosity > 2) {
							cerr << pval << " ";
						}
						if (pval < min_pval) {
							min_pval = pval;
							motif_cutoff = k+1;
						}
					}

					/* the binomial test must produce a p-value good enough
					   otherwise, we will consider the motif as not present */
					if (min_pval > binom_pval) {
						motif_cutoff = 0;
					}
					if (verbosity > 1) {
						cerr << " final pval: " << min_pval << " accept: " << motif_cutoff << " (" << binom_pval << ")" << endl;
					}

					motifs[jj].resize(motif_cutoff);
				} else {
					motif_cutoff = motifs[jj].size();
				}

				// Output everything
				for (size_t k = 0; k < motifs[jj].size(); ++k) {
					Motif_Hit & hit (motifs[jj][k]);
					if (printed_one) {
						fout << "," << endl;
					}
					hit.archive(fout);
					printed_one = true;
				}
				if (motif_cutoff > 0) {
					Motif_Presence p;
					if (binom_pval > 0) {
						p.test = "binomial";
						p.pvalue = min_pval;
					} else {
						p.test = "none";
						p.pvalue = 1;
					}
					p.sequence_name = inpath.string();
					p.pssm_name = g_pssms[jj].get_name();
					p.pssm_accession = g_pssms[jj].get_accession();
					p.count = motif_cutoff;
					p.collected_scores = g_scores_collected[jj];
					if (printed_one) {
						fout << "," << endl;
					}
					p.archive(fout);
					printed_one = true;
				}
				if (verbosity > 1) {
					cerr << "Hits written for " << g_pssms[jj].get_name() << "/" << g_pssms[jj].get_accession() << ": " << motifs[jj].size() << endl;
				}
			}
			fout << "]" << endl;
		} catch (std::exception e) {
			cerr << e.what() << endl;
		}
	}

	r.cleanup_profiles();
}

