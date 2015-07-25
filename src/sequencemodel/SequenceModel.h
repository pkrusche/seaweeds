/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __SequenceModel_H__
#define __SequenceModel_H__

#include <iostream>
#include "datamodel/Serializable.h"

namespace sequencemodel {

	/**
	 * Sequence model interface
	 */
	template <class _string>
	class SequenceModel : public datamodel::Serializable {
	public:
		virtual void learn (std::istream & str) = 0;

		virtual _string generate_sequence (size_t length) = 0;

		virtual void dump (std::ostream & str) = 0;
	};
};

#endif // __SequenceModel_H__
