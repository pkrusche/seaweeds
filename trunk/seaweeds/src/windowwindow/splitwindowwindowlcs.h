/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __SPLITWINDOWWINDOWLCS_H__
#define __SPLITWINDOWWINDOWLCS_H__

#include <vector>

#include "checkpoint/checkpoint.h"

namespace windowlcs {

	template<class _string>
	class PartMatcher : public utilities::Checkpointable {
	public:
		PartMatcher(utilities::Checkpointable * _matcher, _string const & _s) : matcher(_matcher), s(_s) {} 
		~PartMatcher() {
			delete matcher;
		}

		virtual void checkpoint_loadstate(utilities::ParameterFile & p) {
			matcher->checkpoint_loadstate(p);
		}

		virtual void checkpoint_savestate(utilities::ParameterFile & p) {
			matcher->checkpoint_savestate(p);
		}

		virtual void new_checkpointer(utilities::Checkpointer * p) {
			matcher->new_checkpointer(p);
		}
		void run() {
			matcher->run();
		}
	private:
		_string s;
		utilities::Checkpointable * matcher;
	};

	template <class _matcher> 
	class SplitMatcherGenerator {
	public:
		typedef typename _matcher::string string;

		SplitMatcherGenerator(size_t _split_parts = 4) : split_parts(_split_parts) {}

		template <class _reporter >
		std::vector<utilities::Checkpointable*> const & operator()(string const & s1, string const & s2, 
			size_t windowlength, 
			size_t threshold = 0, 
			size_t step1 = 1, 
			size_t step2 = 1,
			_reporter * r = NULL
			) {
			split_computations.resize(split_parts, NULL);
			_matcher m;

			size_t nsteps = ((s1.size() - windowlength + 1 + step1 - 1)/step1 + split_parts - 1) / split_parts;

			for(size_t j = 0; j < split_parts; ++j) {
				string sub = s1.substr(j*nsteps*step1, windowlength+nsteps*step1);
				split_computations[j] = new PartMatcher<string>(
					m(sub, s2, windowlength, threshold, step1, step2, r),
					sub);
			}
			return split_computations;
		}
	private:
		size_t split_parts;
		std::vector<utilities::Checkpointable*> split_computations;
	};
};

#endif
