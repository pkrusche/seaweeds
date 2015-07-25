/***************************************************************************
 *   Copyright (C) 2012 by Peter Krusche                                   *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

/**
 * @file FixedSizeQueue.h
 * @author Peter Krusche
 * 
 */


#ifndef __FixedSizeQueue_H__
#define __FixedSizeQueue_H__

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <queue>
#include <algorithm>

#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

#include "bsp_cpp/bsp_cpp.h"

namespace utilities {

	/** 
	 * heap queue: keeps a fixed maximum number of objects with 
	 * a double-valued key.
	 */
	template<class _value>
	class FixedSizeQueue : public bsp::Reduceable {
	public:
		FixedSizeQueue(
			size_t _max_size = 1 
		) : 
			max_size(_max_size)
		{
			values.reserve(max_size);
		}

		FixedSizeQueue(FixedSizeQueue const & rhs) {
			(*this) = rhs;
		}

		/** enqueue a value with a given key */
		bool enqueue(double key, const _value & val) {
			if (values.size() >= max_size 
			&&  key < top(values).key) {
				/** reject since it's smaller than the min score */
				return false;
			}

			if (values.size() == max_size) {
				pop(values);
			}
			entry_t entry;
			entry.key = key;
			entry.val = val;
			push(values, entry);
			return true;
		}

		/** dump contents to a vector */
		void get_all (std::vector<_value> & target, std::vector<double> * keys = NULL) const {
			target.reserve(values.size());
			if(keys != NULL) {
				keys->reserve(values.size());
			}
			for(typename heap::const_iterator it = values.begin();
				it != values.end();
				++it) {
				if(keys != NULL) {
					keys->push_back(it->key);
				}
				target.push_back(it->val);
			}
		}

		/** get/set the maximum size */
		size_t get_max_size() const { 
			return max_size; 
		}

		/** set max size, reduce number of elements if necessary */
		void set_max_size(size_t val) { 
			while (values.size() > val) {
				pop(values);
			}
			max_size = val; 
			values.reserve(max_size);
		}

		/** get the number of entries stored in this queue */
		inline size_t size() const {
			return values.size();
		}

		/** get the minimum key value that will make it i
		 *  into the queue */
		inline double get_min_key () const {
			return top(values).key;
		}

		/** from bsp::Reduceable */
		void reduce_with(Reduceable const * _rhs) {
			FixedSizeQueue const * rhs = static_cast<FixedSizeQueue const * > (_rhs);

			for(typename heap::const_iterator it = rhs->values.begin();
				it != rhs->values.end();
				++it) {
				enqueue(it->key, it->val);
			}
		}

		/** from bsp::Reduceable */
		void make_neutral() {
			values.clear();
			values.reserve(max_size);
		}

		/** implement operator= to allow initialisation */
		FixedSizeQueue const & operator=(FixedSizeQueue const & rhs) {
			if (&rhs != this) {
				max_size = rhs.max_size;
				values = rhs.values;
			}
			return *this;
		}

		/** from bsp::ByteSerializable */
		void serialize(void * target, size_t nbytes) {
			ASSERT(nbytes >= serialized_size());
			
			size_t * ts = (size_t*) target;

			*ts++ = max_size;
			*ts++ = values.size();

			char * tc = (char *) ts;
			for(typename heap::iterator it = values.begin();
				it != values.end();
				++it) {
					*((double*)tc) = it->key;
					tc+= sizeof(double);
					size_t sz = elem_serialize.serialized_size(it->val);
					*((size_t*)tc) = sz;
					tc+= sizeof(size_t);
					elem_serialize.serialize(it->val, tc, sz);
					tc+= sz;
			}
		}

		/** from bsp::ByteSerializable */
		void deserialize(void * source, size_t nbytes) {
			ASSERT(nbytes >= 3*sizeof(size_t) + 2*sizeof(double));
			
			size_t * ts = (size_t*) source;
			max_size = *ts++;
			make_neutral();
			size_t nentries = *ts++;

			char * tc = (char *) ts;

			for(size_t x = 0; x < nentries; ++x) {
				double key = *((double*)tc);
				tc+= sizeof(double);

				size_t sz = *((size_t*)tc);
				tc+= sizeof(size_t);

				_value val;
				elem_serialize.deserialize(val, tc, sz);
				tc+= sz;

				enqueue(key, val);
			}
		}

		/** from bsp::ByteSerializable */
		size_t serialized_size() {
			size_t sz = 2*sizeof(size_t);

			for(typename heap::iterator it = values.begin();
				it != values.end();
				++it) {
					sz+= sizeof(double);
					sz+= sizeof(size_t);
					sz+= elem_serialize.serialized_size(it->val);
			}
			return sz;
		}

	private:
		/** determine element serializer */
		template <class _t> 
		struct scalar_serialize {
			size_t serialized_size(_t & ) {
				return sizeof(_t);
			}

			void serialize(_t & val, void * target, size_t nbytes) {
				ASSERT(nbytes >= sizeof(_t));
				memcpy(target, &val, sizeof(_t));
			}

			void deserialize(_t & val, void * source, size_t nbytes) {
				ASSERT(nbytes >= sizeof(_t));
				memcpy(&val, source, sizeof(_t));
			}
		};

		template <class _t> 
		struct object_serialize {
			size_t serialized_size (_t & val) {
				return val.serialized_size();
			}

			void serialize(_t & val, void * target, size_t nbytes) {
				val.serialize(target, nbytes);
			}

			void deserialize(_t & val, void * source, size_t nbytes) {
				val.deserialize(source, nbytes);
			}
		};

		typedef typename boost::mpl::if_<
			boost::is_scalar<_value> , 
			scalar_serialize < _value >, 
			object_serialize < _value > > :: type serialize_op;

		serialize_op elem_serialize;

		/** heaping parameters */

		size_t max_size;

		/** entry storage */

		struct entry_t {
			double key;
			_value val;
		};

		struct entry_cmp {
			bool operator() (const entry_t & l, const entry_t & r) {
				return l.key > r.key;
			}
		};

		/** same as std::priority_queue, but we are able to access
		 *  all elements in the queue */
		typedef std::vector<entry_t> heap;

		void push(heap & c, entry_t const & e) {
			c.push_back(e);
			std::push_heap(c.begin(), c.end(), entry_cmp());
		}

		void pop(heap & c) {
			std::pop_heap(c.begin(), c.end(), entry_cmp());
			c.pop_back();			
		}

		entry_t const & top(heap const & c) const	{
			return c.front();
		}

		/** vector for storing values */
		heap values;
	};
};

#endif // __FixedSizeQueue_H__
