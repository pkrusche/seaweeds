/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#include "autoconfig.h"

#include "AlignmentPlot_Method.h"

/** static implementation */
std::map<std::string, boost::shared_ptr<AlignmentPlot_Method_Factory> > 
	AlignmentPlot_Method::methods;

/** factory method : create a new method given an alignment plot & */
boost::shared_ptr<AlignmentPlot_Method> AlignmentPlot_Method::get_method(
	std::string const & name, AlignmentPlot  & ap) {

	if (methods.find(name) == methods.end()) {
		bsp_abort("Unknown method %s", name.c_str());
	}

	return methods[name]->create(ap);
}

