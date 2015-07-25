/***************************************************************************
 *   Copyright (C) 2011   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef __TextInput_H__
#define __TextInput_H__

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>

#include <string.h>

/************************************************************************/
/* We extend std by a few helpers for reading pairs and multiple delims */
/************************************************************************/

namespace std {
/**
 * @brief read from a stream into a string until one of a set of characters is found
 */
inline bool getline(std::istream & i, std::string &str, const char * delim) {
	char c[2];
	c[1] = 0;
	bool cont = true;
	bool success = false;

	while (cont && i.good() ) {
		i.get(c[0]);
		if (i.good() && strchr(delim, c[0]) == NULL) {
			str.append(c);
			success = true;
		} else {
			cont = false;
		}
	}
	
	return success;
}

/**
 * std::pair helpers
 */
template<class _T1, class _T2> std::ostream & operator<<(std::ostream & stream,
		std::pair<_T1, _T2> const & p) {
	stream << "( "<< p.first<< " , "<< p.second<< " )";
	return stream;
}

template<class _T1, class _T2> std::istream & operator>>(std::istream & stream,
		std::pair<_T1, _T2> & p) {
    std::string s;
    
    getline(stream, s, "(" );
	stream >> p.first;
    getline(stream, s, "," );
	stream >> p.second;
    getline(stream, s, ")" );
    
	return stream;
}

/**
 * std::vector helpers
 */
template<class _T, class _A> std::ostream & operator<<(std::ostream & stream,
	std::vector<_T, _A> const & v) {
    using namespace std;
	stream << "[";

	for (typename vector<_T, _A>::const_iterator it = v.begin();
		it != v.end(); ++it) {
			stream << *it << ", ";
		}
/*
 
	TODO: do it like this once MacOS also knows Cxx0x
	std::for_each(v.begin(), v.end(), [&stream] ( _T const & val) {
		stream << val << " , ";
	} );
*/
	stream << "]";
	return stream;
}

};

/************************************************************************/
/* Some magical text I/O functions                                      */
/************************************************************************/

namespace TextIO {

	/************************************************************************/
	/* input/output of multiline strings                                    */
	/************************************************************************/
	inline std::string read_multiline_string(std::istream & is, bool stop_on_empty_line = true) {
		using namespace std;

		string s = "";
		string ss = "";
		while( is.good() && getline(is, ss, '\n') ) {
			if(stop_on_empty_line && ss.length() == 0) {
				break;
			}
			s+= ss;
		}
		return s;
	}		

	inline void print_multiline_string(std::ostream & s, const char * c, size_t linelen = 73) {
		using namespace std;
		char * tmp;
			
		tmp = new char[linelen+1];
        tmp[linelen] = 0;
		size_t len = strlen(c);
		while(len > 0) {
			strncpy(tmp, c, linelen);
			s << tmp << endl;
			c+= min(len, linelen);
			len = strlen(c);
		}
		s << endl;
		delete [] tmp;
	}

	/************************************************************************/
	/* input splitting and trimming (yes, boost has those too)              */
	/************************************************************************/

    inline std::string trim (std::string const & _str, const char * ws = "\n \t\r") {
		std::string str (_str);
		str.erase(str.find_last_not_of(ws)+1);
		std::string::size_type pos = str.find_first_not_of(ws);
		if (pos > 0 && pos != std::string::npos) {
			str.erase(0, pos);
		}	
		return str;
	}

	template < class _list >
	void split (std::string const & str, _list & list, const char* sep = " ", const char * ws = NULL, bool drop_empty = false) {
		using namespace std;
		istringstream is(str);
		string s;
		while (is.good()) {
			s = "";
			getline(is, s, sep);

			if (ws != NULL) {
				s = trim (s, ws);
			}

			if(!drop_empty || s != "" ) {
				list.push_back(s);
			}
		}
		if (getline(is, s, "")) {
			if (ws != NULL) {
				s = trim (s, ws);
			}
			list.push_back(s);
		}
	}

};

#endif // __TextInput_H__
