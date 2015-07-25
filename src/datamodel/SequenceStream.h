/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __SequenceStream_H__
#define __SequenceStream_H__

#include <exception>
#include "../xasmlib/IntegerVector.h"

namespace datamodel {
	
	template <size_t bpc = 2> 
	class SequenceSource {
	public:
		/**
		 * Fill an integer vector with sequence. Does not resize, reads
		 * as much as fits into the vector.
		 */
		virtual size_t read_sequence (utilities::IntegerVector <bpc> & target) = 0;
	};

	template <size_t bpc = 2> 
	class SequenceSink {
	public:
		/**
		 * Write out all the sequence inside this vector
		 */
		virtual void write_sequence (utilities::IntegerVector <bpc> & sequence) = 0;
	};


	/** An input stream of sequence windows.
	 *
	 * This stream returns a potentially long input sequence in 
	 * windows.
	 * 
	 * An optional overlap value specifies how much two consecutive
	 * window queries must overlap.
	 * 
	 */
	template <size_t bpc = 2> 
	class WindowedInputStream {
	public:

		/** Constructor */
		inline WindowedInputStream (size_t _window_overlap) {
			window_overlap = _window_overlap;
			input = NULL;
			current_pos = 0;
			overlap_buffer.resize(window_overlap);
		}

		/** Set the input sequence source
		 * 
		 * @param _input the source
		 * @param reset_position reset the position counter to zero
		 */
		inline void set_input (SequenceSource<bpc> & _input, bool reset_position = true) {
			input = &_input;
			if (reset_position) {
				current_pos = 0;
			}
			remaining_overlap = 0;
		}

		/**
		 * Get the next sequence window.
		 * 
		 * This function returns the next sequence window, which is the suffix of length
		 * window_overlap of the previously returned window, followed by length-window_overlap
		 * characters from the end of the source.
		 * 
		 * @param length the length of the window to get (must be >= window_overlap)
		 * @param target the target sequence
		 * @param position this int gets the current global sequence position
		 * @return the number of bytes that were actually put into target
		 * 
		 */
		size_t get_window (size_t length, utilities::IntegerVector <bpc> & target, int & position) {
			// we cannot go backwards!
			ASSERT (length >= window_overlap);
			ASSERT (input != NULL);
			
			size_t read_len = 0;
			size_t wanted_len = length - window_overlap;
			position = (int) current_pos;

			// try to read length chars
			target.resize(length);

			// if we have sequence already, we need to copy from the overlap buffer
			if (current_pos > 0) {
				other_buffer.resize(wanted_len);
				read_len = input->read_sequence(other_buffer);

				// can copy the first part, target is bigger than overlap_buffer.
				memcpy(	target.datavector().exact_data(0), 
						overlap_buffer.datavector().exact_data(0), 
						overlap_buffer.datavector().exact_size() << 3
				);
				// The memcpy above might have messed this up.
				target.fixending();

				// to support arbitrary bitness, we need to use put/get here
				size_t k;
				for (k = 0; k < read_len; ++ k) {
					target.put(k+window_overlap, 
						(int)other_buffer.get(k));
				}
				while(k < wanted_len) {
					target.put(k+window_overlap, 
						-1);
					++k;
				}

				if (read_len < wanted_len) {
					if (remaining_overlap < 0) {
						remaining_overlap = (long)(window_overlap - (wanted_len - read_len));
						read_len += window_overlap;
					} else {
						using namespace std;
						long int tmp = remaining_overlap;
						remaining_overlap = max ((long int)0l, (long int)(remaining_overlap + read_len - wanted_len));
						read_len = max ((long int)0l, tmp);
					}
				} else {
					read_len+= window_overlap;					
					remaining_overlap = -1;
				}
			} else {
				// initialize overlap buffer
				read_len = input->read_sequence(target);
				// we require that the input contains at least window_overlap chars
				if (read_len < window_overlap) {
					throw std::runtime_error("Input sequence is too short.");
				}
			}
			current_pos+= wanted_len;
			// Get overlap buffer for next step
			target.extract_substring(length - window_overlap, length - 1, overlap_buffer);
			return read_len;
		}

		/** Return the window overlap */
		size_t get_overlap () { return window_overlap; }

		/** Return the current stream position */
		size_t pos () { return current_pos; }

	private:
		size_t window_overlap;	///< minimum overlap
		size_t current_pos;		///< current sequence position
		long int remaining_overlap;			///< when we run out of data, we save the position here
		SequenceSource<bpc> * input;	///< sequence source
		utilities::IntegerVector<bpc> overlap_buffer;	///< overlap buffer
		utilities::IntegerVector<bpc> other_buffer;
	};
};

#endif // __SequenceStream_H__

