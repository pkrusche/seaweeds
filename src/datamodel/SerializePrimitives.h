/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __SERIALIZEPRIMITIVES_H__
#define __SERIALIZEPRIMITIVES_H__ 

#include "Serializable.h"

namespace datamodel {

template <class _ws>
struct read_array {
	typedef typename _ws::value_type value_type;
	void operator() (Json::Value & v, std::vector <value_type> & in) {
		static JSONArray<_ws> arraySerializer;
		arraySerializer.read (v, "", in);
	}	
};

template <class _ws>
struct write_array {
	typedef typename _ws::value_type value_type;
	void operator() (Json::Value & v, std::vector <value_type> const & out) {
		static JSONArray<_ws> arraySerializer;
		arraySerializer.write (v, "", out);
	}	
};

template <class _fun>
struct sread {
	typedef typename _fun :: value_type value_type;
	void operator () (std::istream & read, typename _fun :: value_type & in) {
		static _fun f;
		Json::Value root;
		read >> root;
		f.read ( root, "", in );		
	}
};

template <class _fun>
struct swrite {
	typedef typename _fun :: value_type value_type;
	void operator () (std::ostream & wr, value_type const & in) {
		static _fun f;
		Json::Value root;
		f.write (root, "", in);
		wr << root;
	}
};

};


#endif