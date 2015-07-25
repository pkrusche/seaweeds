/***************************************************************************
 *   Copyright (C) 2011   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef __global_options_H__
#define __global_options_H__

#include <string>
#include "ParameterFile.h"

namespace bsp {
	extern utilities::ParameterFile global_options;

	/*!
	 * \brief read global options files
	 *
	 *
	 */
	static inline void bspcpp_read_global_options() {
		if(bsp_pid() == 0) {
			// check current path, and home directory on unix systems
			global_options.read(".bspcpp_global_params");
#ifndef _WIN32
			global_options.read("~/.bspcpp_global_params");
#endif
		}
		// broadcast, so everyone gets it.
		bsp_broadcast(0, global_options);
	}

	/** helper to get options out */
	template<class _t>
	_t const & global_option(const char * name, _t const & _default) {
		static _t val;
		global_options.get(name, val, _default);
		return val;
	}
};

#endif // __global_options_H__
