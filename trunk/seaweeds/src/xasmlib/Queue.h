/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************


	created:	2009/03/19
	created:	19:3:2009   0:44
	filename: 	xasmlib\Queue.h
	author:		Peter Krusche
	
*********************************************************************/
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <queue>
#include <algorithm>
#include <vector>
#include <iostream>

#include "IntegerVector.h"

namespace utilities {

/**
 * @brief Fast fixed size priority queue class
 */
template <class _t>
class Queue {
public:
	Queue() {}

	virtual void pop(_t v) {
		while(c.size() > 0 && top() > v) {
			pop();
		}
	}

	virtual size_t size() {
		return c.size();
	}

	virtual void dump(std::ostream & o) {
		o << c.size();
		if (c.size() > 0) {
			o << "(";
			for(typename std::vector<_t>::iterator it= c.begin(); it != c.end(); ++it) {
				o << *it << ",";
			}
			o << ")";
		}
	}

	typedef std::vector<_t> container_type;
	typedef typename std::vector<_t>::value_type value_type;
	typedef typename std::vector<_t>::size_type size_type;
	typedef typename std::vector<_t>::reference reference;
	typedef typename std::vector<_t>::const_reference const_reference;

	reference top()
	{	// return mutable highest-priority element (retained)
		return (c.front());
	}

	void push(const value_type& _Pred)
	{	// insert value in priority order
		c.push_back(_Pred);
		push_heap(c.begin(), c.end(), comp);
	}

protected:
	void pop()
	{	// erase highest-priority element
		pop_heap(c.begin(), c.end(), comp);
		c.pop_back();
	}

private:
	std::vector<_t> c;
	std::less<_t> comp;	// the comparator functor
};

};

template <class _t>
inline std::ostream & operator<< (std::ostream & o, utilities::Queue<_t> & q) {
	q.dump(o);
	return o;
}

#endif	
