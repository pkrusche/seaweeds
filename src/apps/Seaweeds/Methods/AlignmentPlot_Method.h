/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __ALIGNMENTPLOT_METHOD_H__
#define __ALIGNMENTPLOT_METHOD_H__

#include "../AlignmentPlot.h"

class AlignmentPlot_Method;

typedef boost::shared_ptr<AlignmentPlot_Method> AlignmentPlot_Method_Ptr;

/** factory base class */
class AlignmentPlot_Method_Factory {
public:
	virtual ~AlignmentPlot_Method_Factory() {}

	virtual AlignmentPlot_Method_Ptr create(AlignmentPlot & ) = 0;
};

/** templated generic factory */
template <class _t> 
class AlignmentPlot_Method_Generic_Factory : public AlignmentPlot_Method_Factory {
public:
	AlignmentPlot_Method_Ptr create(AlignmentPlot & ap) {
		return AlignmentPlot_Method_Ptr(new _t(ap));
	}
};

/** Alignment Plotting interface */
class AlignmentPlot_Method : public windowlocal::window_translator {
public:
	AlignmentPlot_Method(AlignmentPlot & _ap) : ap(_ap) {}

	/** this needs to be implemented to write window matches from 
	 *  s1 vs s2 to ap */
	virtual void run(
		std::string const & s1, 
		std::string const & s2, 
		int offset1 = 0,
		int offset2 = 0) = 0;

	/** factory method : create a new method given an alignment plot & */
	static AlignmentPlot_Method_Ptr get_method(
		std::string const & name, AlignmentPlot  & );

	/** add a method. _apf must be derived from AlignmentPlot_Method_Factory */
	template <class _apf>
	static void add_method(const char * name) {
		methods[std::string(name)] = boost::shared_ptr<AlignmentPlot_Method_Factory>(
			new _apf() );
	}

protected:

	/** The alignment plot we write to */
	AlignmentPlot  & ap; 

private:

	/** factory map -- statically add new methods here */
	static std::map<std::string, boost::shared_ptr<AlignmentPlot_Method_Factory> > methods;
};

#endif
