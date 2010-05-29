/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef ProcMapper_h__
#define ProcMapper_h__

#include <algorithm>
#include <tbb/concurrent_vector.h>
#include <loki/NullType.h>

#include "bsp.h"
#include "tools/utilities.h"

namespace bsp {
	/**
	 * @brief Simple blocked assignment of pids to processors 
	 */

	template <class _context=Loki::NullType>
	class ProcMapper {
	public:
		ProcMapper(int _processors, int _groups = 1) : 
		  processors(_processors*_groups), groups(_groups) {
			using namespace std;
			max_procs_per_node = ICD(processors, ::bsp_nprocs());
			procs_per_group = _processors;
			if( max_procs_per_node * (::bsp_pid()+1) > processors ) {
				procs_on_this_node =  
					max(processors - max_procs_per_node * (signed) ::bsp_pid(), 0);
			} else {
				procs_on_this_node = max_procs_per_node;
			}
			context_store.resize(procs_on_this_node);
		}

		int nprocs () const {
			return processors;
		}

		int ngroups() const {
			return groups;
		}

		int procs_per_node() const {
			return max_procs_per_node;
		}

		int procs_this_node() const {
			return procs_on_this_node;
		}

		int local_to_global_pid(int local_pid) const {
			return ::bsp_pid() * max_procs_per_node + local_pid;
		}

		int local_to_global_group(int local_pid) const {
			return ::bsp_pid() * max_procs_per_node + local_pid;
		}

		int global_to_local_pid(int global_pid, int group = 0) const {
			int p = global_pid - ::bsp_pid() * procs_per_node();
			if (p < 0 || p >= procs_on_this_node)	{
				return -1;
			} else {
				return p;
			}
		}

		_context & get_context(int local_pid) {
			return context_store[local_pid];
		}
		
		_context & get_node_context() {
			return node_context;
		}

	private:
		int processors;
		int procs_on_this_node;
		int max_procs_per_node;
		int groups;
		int procs_per_group;
		
		tbb::concurrent_vector<_context> context_store;
		_context node_context;
	};
};

#endif // ProcMapper_h__
