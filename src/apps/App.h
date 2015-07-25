/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __App_H__
#define __App_H__

#include <iostream>
#include <boost/program_options.hpp>

/** Progress output */
void start_progress( const char * what, int _max );
void add_progress (int p);
void end_progress();

class App {
public:
	
	/** Options for this app to a boost options description */
	inline virtual void add_opts ( boost::program_options::options_description & all_opts ) {}

	/** 
	 * Overload to run app.
	 * 
	 * Extract options from variable map 
	 * 
	 */
	virtual void run(boost::program_options::variables_map & vm) = 0;
};

#endif // __App_H__
