/***************************************************************************
 *   Copyright (C) 2009 by Peter Krusche                                   *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

/**
 * @file WindowPairOutput.h
 * @author Peter Krusche
 * 
 */

#ifndef __WINDOWOUTPUT_H__
#define __WINDOWOUTPUT_H__

namespace windowlocal {

	template <class string>
	class WindowPairOutput {
	public:
		WindowPairOutput (std::ostream & o) : _out(o) {}

		virtual ~WindowPairOutput() {}

		std::ostream & _out;
		double threshold;
		size_t m, n;
		int offset;
		int p_offset;

		double * profile_a;
		double * profile_b;

		size_t profile_size_a;
		size_t profile_size_b;

		size_t stepsize_a;
		size_t stepsize_b;

		virtual void operator() (window<> const & win2) = 0;
	};

	/**
	 * @brief window printer class
	 * This class writes windows into an output stream
	 */
	template <class string, class scoretranslation = lcs::ScoreTranslation<typename string> >
	class WindowOutputUnbuffered : public WindowPairOutput <string> {
	public:
		WindowOutputUnbuffered (
			std::ostream & o, 
			size_t _m, size_t _n, 
			double _t,
			size_t windows_x = 0,
			size_t windows_y = 0,
			size_t step_x = 1,
			size_t step_y = 1
			) : WindowPairOutput <string>(o) {
				m = _m;
				n = _n;
				threshold = _t;
				offset = 0;
				p_offset = 0;

				profile_size_a = windows_x;
				profile_size_b = windows_y;
				stepsize_a = step_x;
				stepsize_b = step_y;

				if (windows_x != 0) {
					profile_a = new double[windows_x];
					memset(profile_a, 0, sizeof(double)*windows_x);
				} else {
					profile_a = NULL;
				}
				if (windows_y != 0) {
					profile_b = new double[windows_y];
					memset(profile_b, 0, sizeof(double)*windows_y);
				} else {
					profile_b = NULL;
				}
		}

		virtual ~WindowOutputUnbuffered() {
			if (profile_a != NULL) {
				delete [] profile_a ;
			}
			if (profile_b != NULL) {
				delete [] profile_b;
			}
		}

		virtual void operator() (window<> const & win2) {
			using namespace std;
			window<> win = win2;
			if(!( // check if this score is reported for a real position, or if it is for a fractional 
				// position (within a supercell)
				scoretranslation::validcoord(win.x0) && scoretranslation::validcoord(win.x1)
				) 
				) {
					return;
			}
			win.x0 = scoretranslation::translatecoord_bk(win.x0);
			win.x1 = scoretranslation::translatecoord_bk(win.x1);
			win.score = scoretranslation::translatescore(m, n, win.score);
			if(win.score > threshold) {
				_out << win.x0 + offset << "\t" << win.x1 << "\t" << win.score << endl;
			}
			if (profile_a != NULL) {
				size_t s_a = (win.x0 + p_offset) / stepsize_a;
				ASSERT(s_a < profile_size_a);
				profile_a[s_a] = max(win.score, profile_a[s_a]);
			}

			if (profile_b != NULL) {
				size_t s_b = win.x1 / stepsize_b;
				ASSERT(s_b < profile_size_b);
				profile_b[s_b] = max(win.score, profile_b[s_b]);
			}
		}

	};

	template <class string, class scoretranslation = lcs::ScoreTranslation<typename string> > 
	class WindowOutputWithBuffer : public WindowOutputUnbuffered < string, scoretranslation > {
	public:
		WindowOutputWithBuffer(
			std::ostream & o, size_t _max_windows,
			size_t _m, size_t _n, 
			double _t,
			size_t windows_x = 0,
			size_t windows_y = 0,
			size_t step_x = 1,
			size_t step_y = 1
			) : WindowOutputUnbuffered <string, scoretranslation> (o, _m, _n, _t, windows_x, windows_y, step_x, step_y) {
			max_windows = _max_windows;
			windowcount = 0;
		}

		virtual ~WindowOutputWithBuffer() {
			using namespace std;
			cout << "Flushing output, window count : " << windowcount << endl;
			for (std::map<int, bucket>::iterator it = winbuffer.begin(); it != winbuffer.end(); ++it) {
				for (size_t s = 0; s < it->second.size(); ++s) {
					WindowPairOutput<string> ::_out << it->second[s].x0 + WindowPairOutput<string> :: offset << "\t" << it->second[s].x1 << "\t" 
						 << it->second[s].score << endl;
				}
			}
			WindowPairOutput<string> ::_out.flush();
		}

		virtual void operator() (window<> const & win2) {
			using namespace std;
			window<> win = win2;
			if(!( // check if this score is reported for a real position, or if it is for a fractional 
				  // position (within a supercell)
				scoretranslation::validcoord(win.x0) && scoretranslation::validcoord(win.x1)
				) 
				) {
				return;
			}
			win.x0 = scoretranslation::translatecoord_bk(win.x0);
			win.x1 = scoretranslation::translatecoord_bk(win.x1);
			win.score = scoretranslation::translatescore( WindowPairOutput<string> ::m,  WindowPairOutput<string> ::n, win.score);
			if(win.score >  WindowPairOutput<string> ::threshold) {
				int h = hashfun(win.score);
				if (winbuffer.find(h) == winbuffer.end()) {
					winbuffer[h] = bucket();
					winbuffer[h].reserve(64);
				}
				bucket & b  = winbuffer[h];
				if (b.size() > b.capacity() - 1) {
					b.reserve(min(b.capacity()*2, max_windows));
				}
				b.push_back(win);
				++windowcount;

				while(windowcount > max_windows) {
					h = hashfun( WindowPairOutput<string> ::threshold );
					if(winbuffer.find(h) != winbuffer.end()) {
						windowcount -= winbuffer[h].size();
						winbuffer.erase(h);
					}
					++ WindowPairOutput<string> :: threshold;
					cout << "Number of windows: " << windowcount << ", threshold raised: " <<  WindowPairOutput<string> ::threshold << endl;
				}
			}
			if ( WindowPairOutput<string> ::profile_a != NULL) {
				size_t s_a = (win.x0 +  WindowPairOutput<string> ::p_offset) /  WindowPairOutput<string> ::stepsize_a;
				ASSERT(s_a <  WindowPairOutput<string> ::profile_size_a);
				WindowPairOutput<string> ::profile_a[s_a] = max(win.score,  WindowPairOutput<string> ::profile_a[s_a]);
			}

			if ( WindowPairOutput<string> ::profile_b != NULL) {
				size_t s_b = win.x1 /  WindowPairOutput<string> ::stepsize_b;
				ASSERT(s_b <  WindowPairOutput<string> ::profile_size_b);
				WindowPairOutput<string> ::profile_b[s_b] = max(win.score,  WindowPairOutput<string> ::profile_b[s_b]);
			}			
		}

		virtual int hashfun(double score) {
			return (int)score;
		}

		typedef struct 	std :: vector < window <> > bucket;

		std::map<int, bucket> winbuffer;
		size_t max_windows;
		size_t windowcount;

	};

};
#endif // __WINDOWOUTPUT_H__
