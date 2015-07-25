/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef __PARAMFILE_H__
#define __PARAMFILE_H__

#include <map>
#include "boost/shared_array.hpp"

namespace utilities {

	extern std::string trimstring(const std::string & g, char c);

	struct Parameter {
		enum ParameterType {
			INT = 0,
			FLOAT = 1,
			STRING = 2,
			EMPTY = 3,
		};

		ParameterType type;
		size_t len;
		boost::shared_array<char> data;
		bool updated;
		double last_update;

		Parameter();
		Parameter(int value);
		Parameter(double value);
		Parameter(std::string const & );
		Parameter(const Parameter & p);

		Parameter & operator=(const Parameter & p);

		std::string as_string();
		int as_int();
		double as_float();

		void read_ascii(std::istream & i);
		void read_binary(std::istream & i);

		void write_ascii(std::ostream & o);
		void write_binary(std::ostream & o);

	};


	class ParameterFile {
	public:

		enum storage_type {
			BINARY = 0,
			ASCII = 1,
		};

		ParameterFile() : prefix(""), verbose_updates(false) {}

		ParameterFile & operator=(ParameterFile const & pf);

		/* 
		read parameters from file.
		*/
		void read(const char * filename, storage_type storage = ASCII);

		/* 
		write parameters to file.
		*/
		void write(const char * filename, storage_type storage = ASCII, bool only_upd = false);

		/* 
		write updated parameters to file.
		*/
		void writeupdate(const char * filename, storage_type storage = ASCII);


		/**
		* @brief this function clears all parameters
		*/
		void clear();

		/*
		Parameter get functions. Will return default value if
		the requested parameter was not supplied in the input 
		file.
		*/
		void get(const char * name, double & val, double def= 0.0);
		void get(const char * name, int & val, int def= 0); 
		void get(const char * name, std::string & val, std::string def= "");

		/*
		Parameter set functions.
		*/
		void set(const char * name, double val);
		void set(const char * name, int val); 
		void set(const char * name, std::string const & val);
		void set(const char * name, Parameter val);

		bool hasValue(const char* name);
		void remove(const char* name);

		/**
		* @brief set the prefix for all variable set/gets
		*/
		void setprefix(const char* prefix = NULL);
		/**
		* @brief get the prefix
		*/
		const char * getprefix();

		void dump(std::ostream & o);

		/**
		 * @brief enable dumping to stdout whenever parameter values are updated.
		 */
		void set_verbose_updates(bool vup);

		/**
		* @brief get reference to contents map
		*/
		std::map<std::string, Parameter> & getcontents();

	protected:
		/**
		 * @brief print a notification about an update to a parameter if verbose_updates=1
		 */
		void notify(const char * pname, Parameter * p, const char * op);

	private:

		typedef std::map<std::string, Parameter>::iterator _iterator;
		std::map<std::string, Parameter> m_Params;
		std::string prefix;
		bool verbose_updates;
	};
};


/**
 * BSP broadcasting.
 */
#include "bsp_broadcast.h"
namespace bsp {
	template <>
	inline void bsp_broadcast(int source, utilities::Parameter & data) {
		using namespace std;
		using namespace utilities;
		bsp_broadcast(source, data.len);
		bsp_broadcast(source, data.last_update);
		bsp_broadcast(source, data.type);
		bsp_broadcast(source, data.updated);

		if (bsp_pid() != source) {
			data.data = boost::shared_array<char>(new char[data.len]);
		}
		::bsp_broadcast(source, data.data.get(), data.len);
	}

	template <>
	inline void bsp_broadcast(int source, utilities::ParameterFile & data) {
		using namespace std;
		using namespace utilities;
		size_t size = data.getcontents().size();
		bsp_broadcast(source, size);

		if (bsp_pid() == source) {
			for (map<string, Parameter>::iterator it = data.getcontents().begin(); it != data.getcontents().end(); ++it) {
				string s1 = it->first; 
				bsp_broadcast(source, s1);
				bsp_broadcast(source, it->second);
			}
		} else {
			for (size_t j = 0; j < size; ++j) {
				string name;
				Parameter value; 
				bsp_broadcast(source, name);
				bsp_broadcast(source, value);
				data.set(name.c_str(), value);
			}
		}
	}
};

#endif // __PARAMFILE_H__
