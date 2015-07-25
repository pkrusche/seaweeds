/***************************************************************************
 *   Copyright (C) 2009 by Peter Krusche                                   *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __ALIGNMENTPLOTIO_H__
#define __ALIGNMENTPLOTIO_H__

#include "datamodel/Serializable.h"
#include "datamodel/TextIO.h"

#include "AlignmentPlot.h"

#include <fstream>

class AlignmentPlotIO : public datamodel::Serializable {
public:

	/** initialise from an alignment plot */
	void init_from (AlignmentPlot & ap) {
		char cc[256]; 
		m = ap.m;
		n = ap.n;
		windowlength = ap.windowlength;

		ap.get_profile_a(profile_1);
		ap.get_profile_b(profile_2);

		std::vector<windowlocal::window> vw;
		ap.windows.get_all(vw);

		for (std::vector<windowlocal::window>::iterator i = vw.begin(); 
			 i != vw.end(); ++i) {
			sprintf(cc, "%i_%i", i->x0, i->x1);
			plot[std::string(cc)] = i->score;
		}
		min_score = ap.windows.get_min_key();

		histogram.resize(ap.output_hist.nbuckets+2);
		scorehist_min = ap.output_hist.min_val;
		scorehist_max = ap.output_hist.max_val;

		for (int i = 0; i < ap.output_hist.nbuckets; ++i) {
			histogram[i+1] = ap.output_hist.buckets[i+2];
		}
		histogram[0] = ap.output_hist.buckets[1];
		histogram[histogram.size()-1] = ap.output_hist.buckets[0];
	}

	/** initialise an alignment plot */
	void assign_to (AlignmentPlot & ap) {
		ap.m = m;
		ap.n = n;
		ap.windowlength = windowlength;
		ap.set_sizes();

		if(ap.profile_a.capacity() != profile_1.size()) {
			throw std::runtime_error("AlignmentPlot IO data is invalid.");
		}
		for (size_t i = 0; 
			i < profile_1.size(); ++i) {
			ap.profile_a[(int)i] = profile_1[(int)i];
		}
		if(ap.profile_b.capacity() != profile_2.size()) {
			throw std::runtime_error("AlignmentPlot IO data is invalid.");
		}
		for (size_t i = 0; 
			i < profile_2.size(); ++i) {
			ap.profile_b[(int)i] = profile_2[(int)i];
		}
		
		ap.set_translator<windowlocal::window_translator> ();
		for(std::map<std::string, double>::iterator it = plot.begin();
			it != plot.end(); 
			++it) {
			std::vector<std::string> s;
			TextIO::split(it->first, s, "_");
			if (s.size() != 2) {
				throw std::runtime_error("AlignmentPlot IO data is invalid.");
			}
			windowlocal::window w(atoi(s[0].c_str()), atoi(s[1].c_str()), 
				it->second);
			ap.windows.enqueue(w.score, w);
		}

		if (histogram.size() < 3) {
			throw std::runtime_error("AlignmentPlot IO data is invalid.");
		}

		ap.output_hist.min_val = scorehist_min;
		ap.output_hist.max_val = scorehist_max;
		ap.output_hist.buckets = boost::shared_array<int>(
			new int[histogram.size()]);
		ap.output_hist.nbuckets = (int)histogram.size()-2;


		for (int i = 0; i < ap.output_hist.nbuckets; ++i) {
			ap.output_hist.buckets[i+2] = histogram[i+1];
		}
		ap.output_hist.buckets[0] = histogram[histogram.size()-1];
		ap.output_hist.buckets[1] = histogram[0];
	}

	/** old-style text output */
	void write_text(const char * prefix) {
		std::string ps(prefix);

		{
			std::ofstream os ( (ps + "_result").c_str());
			os << windowlength << std::endl;
			for(std::map<std::string, double>::iterator it = plot.begin();
				it != plot.end(); 
				++it) {
				std::vector<std::string> s;
				TextIO::split(it->first, s, "_");
				if (s.size() != 2) {
					throw std::runtime_error("AlignmentPlot IO data is invalid.");
				}
				
				os << atoi(s[0].c_str()) << "\t" << atoi(s[1].c_str()) << "\t" << 
					it->second << std::endl;
			}
		}

		{
			std::ofstream os ( (ps + "_profile_1").c_str());		
			for (size_t i = 0; 
				i < profile_1.size(); ++i) {
				os << profile_1[i] << std::endl;
			}
		}

		{
			std::ofstream os ( (ps + "_profile_2").c_str());		
			for (size_t i = 0; 
				i < profile_2.size(); ++i) {
				os << profile_2[i] << std::endl;
			}
		}

		utilities::Histogram h;
		h.min_val = scorehist_min;
		h.max_val = scorehist_max;
		h.buckets = boost::shared_array<int>(
			new int[histogram.size()]);
		h.nbuckets = (int)histogram.size()-2;

		for (int i = 0; i < h.nbuckets; ++i) {
			h.buckets[i+2] = histogram[i+1];
		}

		h.buckets[0] = histogram[histogram.size()-1];
		h.buckets[1] = histogram[0];
		{
			std::ofstream os ( (ps + "_histogram").c_str());
			h.write( os );
		}
		
	}

private:
	/** alignment plot parameters */
	int m, n, windowlength;

	/** minimum score observed */
	double min_score;

	/** in APPLES, we use rounded scores from 0 to max_score 
	 *  This here is a little more general, we give the 
	 *  score range too.
	 */ 
	std::vector<int> histogram;

	/** score histogram description */
	double scorehist_min;
	double scorehist_max;

	/** profile 1  */
	std::vector<double> profile_1;

	/** profile 2  */
	std::vector<double> profile_2;

	/** windows in Perl format (key is string of format [x]_[y]) */
	std::map<std::string, double> plot;

	JSONIZE(AlignmentPlotIO, 0, 
		S_STORE( m, datamodel::JSONInt<>  )
		S_STORE( n, datamodel::JSONInt<>  )
		S_STORE( windowlength, datamodel::JSONInt<>  )
		S_STORE( min_score, datamodel::JSONDouble<>  )
		S_STORE( histogram, datamodel::JSONArray < datamodel::JSONInt<> > )
		S_STORE( scorehist_min, datamodel::JSONDouble<>  )
		S_STORE( scorehist_max, datamodel::JSONDouble<>  )
		S_STORE( profile_1, datamodel::JSONArray < datamodel::JSONDouble<> > )
		S_STORE( profile_2, datamodel::JSONArray < datamodel::JSONDouble<> > )
		S_STORE( plot, datamodel::JSONMap < datamodel::JSONDouble<> > )
	);

};

#endif /* __ALIGNMENTPLOTIO_H__ */
