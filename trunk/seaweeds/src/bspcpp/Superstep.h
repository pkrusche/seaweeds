/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef bsp_cpp_superstep_h__
#define bsp_cpp_superstep_h__

#include <string>
#include <string.h>

#ifndef ASSERT
#define ASSERT(x)
#endif

#include <tbb/task.h>
#include <tbb/mutex.h>
#include <tbb/atomic.h>
#include <tbb/tbb_thread.h>

#include "loki/NullType.h"

#include "bspcpp/tools/singletons.h"
#include "bspcpp/ParameterFile.h"
#include "bspcpp/ProcMapper.h"
#include "bsp_global_drma.h"

namespace bsp {
	/**
	 * @brief Class template for BSP computation. 
	 * 
	 * BSP computation implementations must be derived from this class to
	 * make sure the correct overloaded methods are used (i.e. the compuation
	 * runs using threads and not nodes.
	 */
	template <class _context = Loki::NullType>
	class DefaultSuperstep {
	public:
		typedef _context context_t;
		virtual void run() = 0;

	protected:
		virtual int bsp_nprocs() const = 0;
		virtual int bsp_pid() const = 0;
		virtual int bsp_local_pid() const = 0;
		virtual _context & get_context() = 0;
		virtual _context & get_node_context() = 0;

		/** @name thread-safe global DRMA */
		/*@{*/
		virtual void bsp_global_get(bsp_global_handle_t src, size_t offset, 
			void * dest, size_t size) = 0;
		virtual void bsp_global_put(const void * src, bsp_global_handle_t dest, 
			size_t offset, size_t size) = 0;
		virtual void bsp_global_hpget(bsp_global_handle_t src, size_t offset, 
			void * dest, size_t size) = 0;
		virtual void bsp_global_hpput(const void * src, bsp_global_handle_t dest, 
			size_t offset, size_t size) = 0;
		/*@}*/
	};


	/**
	 * @brief This class runs a specific computation pid in a tbb task
	 */
	template <class _computation>
	class ComputationTask : public tbb::task {
	public:
		ComputationTask(_computation * __computation, bool _destroy = false) : 
			computation(__computation), destroy (_destroy) {}

		tbb::task * execute() {
			computation->run();
			if (destroy) {
				delete computation;
			}
			return NULL;
		}
		bool destroy;
		_computation * computation;
	};

	/**
	 * @brief main BSP computation class
	 * 
	 *  To implement single superstep computations, subclass and overload run().
	 */
	template <
		class _implementation, 
		template <class> class _procmapper = ProcMapper
	>
	class Superstep : public _implementation {
	public:
		typedef _procmapper<typename _implementation::context_t> procmapper_t;
		typedef Superstep <_implementation, _procmapper
			 > my_type;

		Superstep(Superstep const & c, int _local_pid) : _implementation(c), 
			procmapper(c.procmapper) {
			if(!singletons::task_scheduler_init.is_active()) {
				singletons::task_scheduler_init.initialize();
			}
			root_task = c.root_task;
			local_pid = _local_pid;
			pid = procmapper.local_to_global_pid(local_pid);
			firstone = false;
		}
		Superstep(procmapper_t & pm) : procmapper(pm) {
			if(!singletons::task_scheduler_init.is_active()) {
				singletons::task_scheduler_init.initialize();
			}
			root_task = NULL;
			firstone = false;
			local_pid = 0;
			pid = procmapper.local_to_global_pid(local_pid);
		}

		Superstep(Superstep * _pred, procmapper_t & pm) : _implementation(*_pred), 
			procmapper(pm) {
			if(!singletons::task_scheduler_init.is_active()) {
				singletons::task_scheduler_init.initialize();
			}
			pred = _pred;
			root_task = pred->root_task;
			firstone = false;
			local_pid = 0;
			pid = procmapper.local_to_global_pid(local_pid);
		}

		void start() {
			ASSERT(procmapper.procs_this_node() != NULL);
			ComputationTask<my_type> *tsk = NULL;
			if(root_task == NULL) {
				// normally, we create a root task which will create child tasks for
				// all the other local processors
				ComputationTask<my_type> *tsk = NULL;
				int nprocs = procmapper.procs_this_node();

				firstone = true;
				dummy_task = new (tbb::task::allocate_root()) tbb::empty_task;
				dummy_task-> set_ref_count(nprocs+1);

				tsk = new (dummy_task->allocate_child()) ComputationTask<my_type>(this);
				dummy_task->spawn(*tsk);
				root_task = tsk;

				for (int t = 1; t < nprocs; ++t) {
					tsk = new ( dummy_task->allocate_child() ) 
						ComputationTask<my_type>(new my_type(*this, t), true);
					dummy_task->spawn(*tsk);
				}
			} else {
				// if we already have a root task, we assume that we are recursively splitting
				// downwards without the need to synchronize before
				dummy_task = root_task;
				ComputationTask<my_type> *tsk = NULL;
				int nprocs = procmapper.procs_this_node();

				firstone = false;
				dummy_task-> set_ref_count(nprocs+1);

				tsk = new (dummy_task->allocate_child()) ComputationTask<my_type>(this);
				dummy_task->spawn(*tsk);
				root_task = tsk;

				for (int t = 1; t < nprocs; ++t) {
					tsk = new ( dummy_task->allocate_child() ) 
						ComputationTask<my_type>(new my_type(*this, t), true);
					dummy_task->spawn(*tsk);
				}
			}
		}

		void join() {
			if(dummy_task != NULL) {
				dummy_task->wait_for_all();
				if(firstone) {
					dummy_task->destroy(*dummy_task);
				}
			}
			dummy_task = NULL;
		}
		
		bool is_root() {
			return local_pid == 0;
		}

		procmapper_t & get_procmapper() {
			return procmapper;
		}
	protected:
		
		virtual int bsp_nprocs() const {
			return procmapper.nprocs();
		}

		virtual int bsp_pid() const {
			return pid;
		}

		virtual int bsp_local_pid() const {
			return local_pid;
		}

		virtual typename _implementation::context_t & get_context() {
			return get_procmapper().get_context(local_pid);
		}

		virtual typename _implementation::context_t & get_node_context() {
			return get_procmapper().get_node_context();
		}

		virtual void bsp_global_get(bsp_global_handle_t src, size_t offset, 
			void * dest, size_t size) {
			tbb::mutex::scoped_lock l(singletons::global_mutex);
			::bsp_global_get(src, offset, dest, size);
		}

		virtual void bsp_global_put(const void * src, bsp_global_handle_t dest, 
			size_t offset, size_t size) {
			tbb::mutex::scoped_lock l(singletons::global_mutex);
			::bsp_global_put(src, dest, offset, size);
		}

		virtual void bsp_global_hpget(bsp_global_handle_t src, size_t offset, 
			void * dest, size_t size) {
			tbb::mutex::scoped_lock l(singletons::global_mutex);
			::bsp_global_hpget(src, offset, dest, size);
		}

		virtual void bsp_global_hpput(const void * src, bsp_global_handle_t dest, 
			size_t offset, size_t size) {
			tbb::mutex::scoped_lock l(singletons::global_mutex);
			::bsp_global_hpput(src, dest, offset, size);
		}

	private:
		Superstep * pred; ///< predecessor superstep
		tbb::task * root_task;	///< root task for all children
		tbb::task * dummy_task;	///< dummy task for spawning the root
		procmapper_t & procmapper;  ///< The process mapper object
		int pid; ///< The global pid of this computation
		int local_pid; ///< The local pid of this computation
		bool firstone; ///< indicates whether we are the first task
	};

};


#endif // bsp_cpp_superstep_h__
