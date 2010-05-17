/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef RANGETREE_H_
#define RANGETREE_H_

#include <loki/SmartPtr.h>
#include <algorithm>
#include <map>
#include <string>

#include "bspcpp/tools/utilities.h"
#include "util/rs_container.h"
#include "IRange.h"
#include "BinTree.h"

namespace rangesearching {

template <
	class _value_type,
	class _comparison_fun,
	class _add_fun,
	class _leaf_type = PrecomputeSum<_value_type, _add_fun>,  
	size_t _cutoff = 0,
	bool presorting = true
> class RangeTree : public IRange<_value_type, rs_container< _value_type >, _comparison_fun, _add_fun, true> {
	public:
	
	typedef typename _add_fun::result _query_type;
	typedef rs_container< _value_type > _container_type;
	typedef typename _container_type::iterator iterator;
	typedef IRange<_value_type, _container_type, _comparison_fun, _add_fun, true> _base_type;
	typedef RangeTree<_value_type, _comparison_fun, _add_fun, _leaf_type, _cutoff, presorting> _RangeTree;

	class NoPresortingInitializer {
	public:
		void operator()(_RangeTree & rthis, iterator _begin, iterator _end, bool) {
			using namespace std;
			size_t n = _end - _begin;

			if (n == 0) {
				return;
			}

			bool fmin = false;
			bool fmax = false;
			/* find min and max over this tree */
			for (iterator it = _begin; it != _end; ++it) {
				if(!fmin || rthis._less(*it, rthis.min)) {
					fmin = true;
					rthis.min = *it;
				}
				if(!fmax || rthis._less(rthis.max, *it)) {
					fmax = true;
					rthis.max = *it;
				}
			}

			/* build subtrees */
			/* find median coordinate */
			size_t A_size = ((n+1) >> 1);
			size_t B_size = n - A_size;

			if ( ( A_size != 0 && B_size != 0 ) 
				 && n > _cutoff
			   ) {
				iterator mid_it = rthis.begin() + A_size;
				nth_element(rthis.begin(), mid_it, rthis.end(), rthis._less);
				rthis.mid = *mid_it;

				rthis.A = new _RangeTree(rthis.begin(), mid_it);
				rthis.B = new _RangeTree(mid_it, rthis.end());
			} else {
				rthis.is_leaf = true;
			}
			rthis._data = new _leaf_type(rthis.begin(), rthis.end());
		}
	};

	class PresortingInitializer {
	public:
		void operator()(_RangeTree & rthis, iterator _begin, iterator _end, bool presorted) {
			using namespace std;
			size_t n = _end - _begin;

			if (n == 0) {
				return;
			}

			if(!presorted) {
				sort(rthis.begin(), rthis.end(), rthis._less);
			}

			rthis.min= *(rthis.begin());
			iterator i = rthis.end();
			rthis.max= *(--i);

			/* build subtrees */
			/* find median coordinate */
			size_t A_size = ((n+1) >> 1);
			size_t B_size = n - A_size;

			if ( ( A_size != 0 && B_size != 0 ) 
				 && n > _cutoff
			   ) {
				iterator mid_it = rthis.begin() + A_size;
				rthis.mid = *mid_it;

				rthis.A = new _RangeTree(rthis.begin(), mid_it, true);
				rthis.B = new _RangeTree(mid_it, rthis.end(), true);
			} else {
				rthis.is_leaf = true;
			}
			rthis._data = new _leaf_type(rthis.begin(), rthis.end());

		}
	};

	
	typedef typename Loki::Select<presorting, 
								  PresortingInitializer, 
								  NoPresortingInitializer >::Result Initializer;

	RangeTree(_container_type & container, bool presorted = false) : 
		_base_type(container.begin(), container.end()),	A(NULL), B(NULL), _data(NULL), is_leaf(false) 
	{
		Initializer i;
		i(*this, _base_type::begin(), _base_type::end(), presorted);
	}

	RangeTree(iterator _begin, iterator _end, bool presorted = false) :
		_base_type(_begin, _end), A(NULL), B(NULL), _data(NULL), is_leaf(false)  
	{
		Initializer i;
		i(*this, _base_type::begin(), _base_type::end(), presorted);
	}


	_query_type & query(const _value_type & p1, const _value_type & p2, _query_type & result) {		
		if ( _less(max, p1) || _less(p2, min) ) { // not contained
			return result;
		} else if ( (!_less(min, p1)) && (!_less(p2, max)) ) { // fully contained
			return _data->query(p1, p2, result); 
		} else if (!is_leaf) { // partially contained
			if ( !_less(mid, p1) ) {
				result = A->query(p1, p2, result);
			}

			if ( !_less(p2, mid) ) {
				result = B->query(p1, p2, result);
			}
		}
		return result;
	}

	void dump(int level = 0) {
		_base_type::dump(level);
		std::string s;
		for(int l = 0; l < level; ++l) {
			s+= "\t";
		}
		if(!is_leaf) {
			std::cout << s << "A= " << std::endl;
			A->dump(level+1);
			std::cout << s << "B= " << std::endl;
			B->dump(level+1);
		}
	}
	
private:
	Loki::SmartPtr< _RangeTree, Loki::RefCounted, Loki::AllowConversion, Loki::NoCheck > A, B; ///< the two subtrees
	Loki::SmartPtr< _leaf_type > _data; ///< query structure for this node's data elements
	
	_value_type max;
	_value_type min;
	_value_type mid;

	bool is_leaf;
};
};


		 
#endif /*RANGETREE_H_*/
