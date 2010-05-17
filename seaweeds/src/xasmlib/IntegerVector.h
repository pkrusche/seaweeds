/***************************************************************************
*   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
*   peter@dcs.warwick.ac.uk                                               *
***************************************************************************/
// Created on 18 January 2008, 21:25
//

#ifndef _MACHINEWORD_H
#define	_MACHINEWORD_H

#include <iostream>
#include <iomanip>
#include <boost/mpl/assert.hpp>
#include <algorithm>

#include "bspcpp/Avector.h"
#include "../xasmlib/xasmlib.h"

extern "C" UINT64 bitmasks_64[];

#ifdef _USE_ASMLIB
#include "../libs/asmlib/asmlib.h"

#define memset A_memset
#define memcpy A_memcpy

#endif

namespace utilities {

	/**
	* \brief Proxy class for implementing operator[] in IntegerVector
	*/
	template <BYTE value_bits>
	class UINT64_proxy {
	public:
		/**
		* \brief initialize using position and a bit offset (which is < 64)
		*/
		UINT64_proxy(UINT64 * pos, BYTE _bitofs) {
			w = pos;
			bitofs = _bitofs;
		}

		operator UINT64() const {
			return extractword(w, bitofs, value_bits);
		}

		UINT64_proxy & operator=(UINT64 _value) {
			insertword(_value, w, bitofs, value_bits);
			return *this;
		}

		UINT64_proxy & operator=(UINT64_proxy const & _value) {
			insertword((UINT64)_value, w, bitofs, value_bits);
			return *this;
		}
	private:
		UINT64 * w;
		BYTE bitofs;
	};

	// specialize proxy for BYTE, WORD, DWORD and UINT64
	template <>
	UINT64_proxy<8>::operator UINT64() const {
		return ((BYTE*)w)[(bitofs>>3)];
	}

	template <>
	UINT64_proxy<8> & UINT64_proxy<8>::operator=(UINT64 _value) {
		((BYTE*)w)[(bitofs>>3)] = (BYTE) (_value & 0xff);
		return *this;
	}

	template <>
	UINT64_proxy<16>::operator UINT64() const {
		return ((WORD*)w)[(bitofs>>4)] & 0xffff;
	}

	template <>
	UINT64_proxy<16> & UINT64_proxy<16>::operator=(UINT64 _value) {
		((WORD*)w)[(bitofs>>4)] = (WORD)(_value & 0xffff);
		return *this;
	}

	template <>
	UINT64_proxy<32>::operator UINT64() const {
		return ((DWORD*)w)[(bitofs>>5)];
	}

	template <>
	UINT64_proxy<32> & UINT64_proxy<32>::operator=(UINT64 _value) {
		((DWORD*)w)[(bitofs>>5)] = (DWORD)(_value);
		return *this;
	}

	template <>
	UINT64_proxy<64>::operator UINT64() const {
		return *w;
	}

	template <>
	UINT64_proxy<64> & UINT64_proxy<64>::operator=(UINT64 _value) {
		*w  = _value;
		return *this;
	}

	/**
	* \brief Vector of "Machine Words", i.e. fixed bit length integers.
	* 
	* We provide an efficient implementation of various functions that are a 
	* little tricky to implement efficiently in standard C++.
	*/
	template <BYTE value_bits>
	class IntegerVector {
	public:
		static const UINT64 msb = (static_cast<UINT64>(1)) << (value_bits - 1);
		static const UINT64 lsbs = 2*msb - 1;

		typedef IntegerVector my_type;
		typedef UINT64 value_type;

		/**
		* Default constructor
		*/
		IntegerVector(size_t words = 0) : 
		vword_len(words), is_slice(false) {
			value_mask = bitmasks_64[value_bits];
			_carry = false;
			content.resize(vwords_toUINT64s(words));
			fixending();
		}

		/**
		 * Copy constructor
		 */
		IntegerVector(my_type const & v) : content(v.content), vword_len(v.vword_len), _carry(v._carry), is_slice(false) {
			value_mask = bitmasks_64[value_bits];
			fixending();
		}

		/**
		 * Construct from given array of 64 bit integers. Vectors constructed like this 
		 * can have their data domain changed dynamically (take care with this, as this
		 * essentially is a way around bounds checking).
		 */
		IntegerVector(my_type & data, size_t _vword_len) : 
			content(data.content, 0, vwords_toUINT64s(_vword_len)), 
			_carry(false), vword_len(_vword_len), is_slice(true) {
			value_mask = bitmasks_64[value_bits];
		}

		/**
		 * Construct from a string. Note that this function translates the input string
		 * lexicographically into numbers: character 'a' is translated into 1, 'b' into 2, 
		 * and so on. Every character < 'A' becomes a 0.
		 */
		IntegerVector(const char * string) :  is_slice(false) {
			vword_len = strlen(string);
			value_mask = bitmasks_64[value_bits];
			_carry = false;

			if(vword_len > 0) {
				content.resize(vwords_toUINT64s(vword_len));
				fixending();
				for(size_t j = 0; j < vword_len; ++j) {
					if(string[j] >= 'A') {
						(*this)[j] = toupper(string[j]) - 'A' + 1;
					} else {
						(*this)[j] = 0;
					}
				}
			}
		}

		/**
		 * Construct from std::string
		 */
		IntegerVector(std::string const & s) : 
		vword_len(s.size()), is_slice(false) {
			value_mask = bitmasks_64[value_bits];
			_carry = false;
			content.resize(vwords_toUINT64s(vword_len));
			for(size_t j = 0; j < vword_len; ++j) {
				(*this)[j] = s[j] & value_mask;
			}
			fixending();
		}

		/**
		* \section Initialisation and element access.
		*/ 

		/**
		* reset all words to zero
		*/
		void zero() {
			if(vword_len > 0) {
				memset(content.data, 0, sizeof(UINT64)*content.size);
			}
		}

		/**
		 * reset all words to -1 (having all bits set)
		 */
		void one() {
			if(vword_len > 0) {
				memset(content.data, 0xff, sizeof(UINT64)*content.size);
				fixending();
			}
		}

		/**
		 * \brief reverse elements of this vector
		 */
		void reverse() {
			for(size_t j = 0; j < vword_len/2; ++j) {
				UINT64_proxy<value_bits> tmp1 = (*this)[j];
				UINT64_proxy<value_bits> tmp2 = (*this)[vword_len - j - 1];
				UINT64 itmp1= tmp1;
				UINT64 itmp2 = tmp2;
				tmp1 = itmp2;
				tmp2 = itmp1;
			}
		}

		/**
		 * \brief append another vector
		 */
		void append(my_type const & a) {
			size_t l = vword_len;
			resize(vword_len + a.vword_len);

			for (size_t j = l; j < vword_len; ++j) {
				this->put(j, a.get(j - l));
			}
		}

		/**
		* element access
		*/
		UINT64_proxy<value_bits> operator[] (size_t ofs) const {
			ASSERT(ofs < vword_len);
			return UINT64_proxy<value_bits>(content.data + vwords_toUINT64s_lb(ofs), bitofs(ofs) );
		}

		void put(size_t pos, int value) {
			insertword(value, content.data + vwords_toUINT64s_lb(pos), bitofs(pos), value_bits);
		}

		int get(size_t pos) const {
			return (int)extractword(content.data + vwords_toUINT64s_lb(pos), bitofs(pos), value_bits);
		}

		/**
		* resize the vector, preserving contents up to given size.
		*/
		void resize(size_t words) {
			if(words == vword_len) {
				return;
			}
			if(!is_slice) {
				vword_len = words;
				content.resize(vwords_toUINT64s(words));
			} else {
				// WARNING: this is an unchecked operation. use with caution
				content.size = vwords_toUINT64s(words);
				vword_len = words;
			}
		}

		/**
		* Generate a mask of matches: set (*this)[j] = -1 iff. s1[j] == s2[j]
		*/
		void generate_match_mask(const my_type & s1, const my_type & s2) {
			ASSERT(s1.size() == s2.size() && s1.size() == size());
			for (size_t j = 0; j < vword_len; ++j) {
				if (s1[j] == s2[j]) {
					(*this)[j] = (UINT64)-1;
				} else {
					(*this)[j] = 0;
				}
			}
		}

		/**
		* \section Element-wise binary operations with single machine word parameter
		*/ 

		my_type & operator+= (UINT64 j) {
#ifndef _X86_64
			vecadd_elemwise(content.data, content.size, j, value_bits);
			fixending();
#else
			for(size_t x = 0; x < vword_len; ++x) {
				(*this)[x] = (*this)[x] + j;
			}
#endif
			return *this;
		}

		my_type & operator-= (UINT64 j) {
#ifndef _X86_64
			vecsub_elemwise(content.data, content.size, (DWORD)j, value_bits);
			fixending();
#else
			for(size_t x = 0; x < vword_len; ++x) {
				(*this)[x] = (*this)[x] - j;
			}
#endif
			return *this;
		}

		my_type & operator&= (UINT64 j) {
#ifndef _X86_64
			vecand_elemwise(content.data, content.size, (DWORD)j, value_bits);
			fixending();
#else
			for(size_t x = 0; x < vword_len; ++x) {
				((*this)[x]) = ((*this)[x]) & j;
			}
#endif
			return *this;
		}

		my_type & operator|= (UINT64 j) {
#ifndef _X86_64
			vecor_elemwise(content.data, content.size, (DWORD)j, value_bits);
			fixending();
#else
			for(size_t x = 0; x < vword_len; ++x) {
				((*this)[x]) = ((*this)[x]) | j;
			}
#endif
			return *this;
		}

		my_type & operator^= (UINT64 j) {
#ifndef _X86_64
			vecxor_elemwise(content.data, content.size, (DWORD)j, value_bits);
			fixending();
#else
			for(size_t x = 0; x < vword_len; ++x) {
				((*this)[x]) = ((*this)[x]) ^ j;
			}
#endif
			return *this;
		}

		void set_bits(UINT64 bits) {
			*this |= (UINT64) bits;
		}

		void and_bits(UINT64 bits) {
			*this &= (UINT64)bits;
		}

		void reset_bits(UINT64 bits) {
			bits = ~(bits & ( ((static_cast<UINT64>(1)) << value_bits) - 1) );
			*this &= (UINT64)bits;
		}


		/**
		* \section Element-wise binary operations with other vector
		*/ 

		/**
		* assignment operator. Copies all elements in v into this vector
		*/
		my_type & operator= (my_type const& v) {
			resize(v.vword_len);
			MEMCPY(content.data, v.content.data, content.size*sizeof(UINT64));
			_carry= v._carry;		
			vword_len = v.vword_len;
			fixending();
			return *this;
		}


		/**
		* element-wise addition
		*/
		my_type & operator+= (my_type const& v) {
			ASSERT(content.size == v.content.size);
			_carry = false;
			_carry = vecadd(content.data, v.content.data, last_relevant()+1, _carry) != 0;

			if( (vword_len*value_bits & 0x3f) != 0) {
				// mask for bit after last word
				UINT64 mask = bitmasks_64[last_bit_ofs()];
				_carry = (content.data[last_relevant()] & mask) != 0;
			}
			fixending();
			return *this;
		}

		my_type & operator-= (my_type const& v) {
			ASSERT(content.size == v.content.size);
			_carry = false;
			_carry = vecsub(content.data, v.content.data, last_relevant()+1, _carry) != 0;

			if( (vword_len*value_bits & 0x3f) != 0) {
				// mask for bit after last word
				UINT64 mask = bitmasks_64[last_bit_ofs()];
				_carry = (content.data[last_relevant()] & mask) != 0;
			}
			fixending();
			return *this;
		}

		my_type & operator&= (my_type const& v) {
			ASSERT(content.size == v.content.size);
			vecand(content.data, v.content.data, content.size);
			return *this;
		}

		my_type & operator|= (my_type const& v) {
			ASSERT(content.size == v.content.size);
			vecor(content.data, v.content.data, content.size);
			return *this;
		}

		my_type & operator^= (my_type const& v) {
			ASSERT(content.size == v.content.size);
			vecxor(content.data, v.content.data, content.size);
			return *this;
		}

		my_type & operator<<= (int shift) {
#ifndef _HAVE_SSE2
			vecshl(content.data, (size_t)shift, content.size);
#else
			vecshl_mmx(content.data, (size_t)shift, content.size);
#endif
			fixending();
			return *this;
		}

		my_type & operator>>= (int shift) {
			fixending();
			vecshr(content.data, (size_t)shift, content.size);
			return *this;
		}

		/**
		* compare-exchange: exchanges elements (*this)[i] and t[i] iff. 
		* (*this)[i] > t[i]
		*/
		void cmpxchg(my_type & t) {
			using namespace std;
			ASSERT(vword_len == t.vword_len);
			for (size_t j= 0; j < vword_len; ++j) {
				UINT64 tmp1 = (*this)[j];
				UINT64 tmp2 = t[j];
				if (tmp1 > tmp2) {
					(*this)[j] = tmp2;
					t[j] = tmp1;
				}
			}
		}

		/**
		* replace bits in this vector with bits from v only if they are set in the mask
		*/
		void replace_if(my_type const & mask, my_type const & vy) {
			ASSERT(content.size == vy.content.size && content.size == mask.content.size);
#ifndef _HAVE_SIMD
			using namespace std;
			size_t minlen = min(mask.vword_len, min(vword_len, vy.vword_len));
			for (size_t j= 0; j < minlen; ++j) {
				UINT64 x = (*this)[j];
				UINT64 y = vy[j];
				UINT64 m = mask[j];

				(*this)[j] = ( x & (~m) ) | (y & m);
			}
#else
			utilities::replace_if_mmx(content.data, vy.content.data, mask.content.data, content.size);
#endif
		}

		/**
		* bit-parallel LCS computation kernel. computes (*this) = ( (*this) & v ) + ( (*this) & ~v )
		*/
		my_type & add_cipr(my_type const & v, BYTE carry = 0) {
			ASSERT(content.size == v.content.size);
			_carry = vecadd_cipr(v.content.data, content.data, last_relevant()+1, carry) != 0;

			if( (vword_len*value_bits & 0x3f) != 0) {
				// mask for bit after last word
				UINT64 mask = bitmasks_64[last_bit_ofs()];
				_carry = (content.data[last_relevant()] & mask) != 0;
			}
			fixending();
			return *this;
		}

		/** retrieve carry after addition */
		bool carry() const {
			return _carry;
		}

		/**
		* \section Substring extraction
		*/

		/**
		* Extract characters (*this)[start, end] (range including start and end)
		*/
		void extract_substring(size_t start, size_t end, my_type & target) const {
			using namespace std;
			if(start > end) {
				return;
			}
			// we might need to shift right an entire entry
			size_t tsize = end-start + (128/value_bits);
			if(target.size() < tsize) {
				target.resize(tsize);
			}
			target.zero();
			size_t start_cpy = (start*value_bits) >> 6;
			size_t real_copylen = min( content.size - start_cpy + 1, target.content.size);
			MEMCPY(target.content.data, content.data + start_cpy, real_copylen*sizeof(UINT64));
			target>>= (BYTE) ((start*value_bits) & 0x3f);
			target.vword_len = end-start+1;
		}

		/**
		* Compatibility with std::string::substr. extract length characters, starting at position
		*/
		my_type substr(size_t position, size_t length) const {
			using namespace std;
			my_type t((size_t)length);

			extract_substring(max((int)position, 0), min(position+length-1, vword_len - 1), t);
			return t;
		}

		/**
		* \section special functions
		*/

		/**
		* test for each word if a specific bit is set. sets each word that has the bit set to -1, all others
		* to 0
		*/
		void bittest(int c = 0) {
			const UINT32 tb = ((static_cast<UINT64>(1)) << c);		
			(*this) &= tb;
			if(c > 0) {
				(*this) >>= c;
			}
			(*this) |= msb;
			(*this) -= 1;
			(*this) ^= msb-1;
		}

		/**
		* saturated increment, only increments each word if not all bits are set already
		*/
		void saturated_inc() {
			my_type tmp(*this);
			my_type topbits(*this);
			(*this) &= lsbs;
			(*this) += 1;

			tmp&= (*this);
			tmp.bittest(value_bits-1);
			(*this) |= tmp;

			topbits&= msb;
			(*this) |= topbits;
		}

		/**
		* sets all words to -1 which are equal to zero, resets all other words
		*/
		void test_zero() {
			my_type tmp(*this);
			tmp -= 1;
			(*this) ^= tmp;
			bittest(msb);
		}

		/**
		* inplace bitwise negation 
		*/
		void negate() {
			for(size_t j = 0; j < content.size; ++j) {
				content.data[j] = ~content.data[j];
			}
			fixending();
		}

		/**
		* bitwise negation and copy
		*/
		my_type operator~() const {
			my_type neg_me(*this);
			for(size_t j = 0; j < neg_me.content.size; ++j) {
				neg_me.content.data[j] = ~neg_me.content.data[j];
			}
			neg_me.fixending();
			return neg_me;
		}

		/**
		* count all bits which are set all words in the entire vector.
		*/
		size_t count_bits() {
			fixending();
			return (size_t) countbits(content.data, last_relevant()+1);
		}

		/**
		* count all bits which are not set all words in the entire vector.
		*/
		size_t count_zeros() {
			return vword_len * value_bits - count_bits();
		}

		/**
		* return the number of words in this vector
		*/
		size_t size() const {
			return vword_len;
		}

		/**
		* direct data access pointer
		*/
		char * data() const {
			return (char*)content.data;
		}

		/**
		* direct data access size
		*/
		size_t datasize() const {
			return content.size << 3;
		}

		/**
		* move the start of the data window of the vector. only valid if it has
		* been constructed from raw data and if value_bits evenly divides 64 bits.
		* this is only a stub, implementation is in MachineWordVectorFixed.h
		* @param offset: offset for start in 64-bit words
		*/
		void shiftstart(int offset) {
			ASSERT(64%value_bits == 0);
			ASSERT(!content.data_is_mine);
			content.data = content.data + offset;
			content.size-= offset;
			vword_len -= (offset * (64/value_bits)); // here we don't want to have vwords crossing the QWORD boundary
		}

	private:
		BYTE last_bit_ofs() {
			return (BYTE) (((vword_len)*((size_t)value_bits)) & 0x3f);
		}

		size_t last_relevant() {
			return vwords_toUINT64s_lb(vword_len);
		}

		/** After shifting, there might be some bits left after the ending.
		*  We remove them here. */
		void fixending() {
			fixending_64(content.data, value_bits, (DWORD)vword_len, (DWORD)content.size);
		}

		/** number of virtual words to machine words (ceiling) */
		size_t vwords_toUINT64s(size_t lwords) const {
			return (size_t) ( ( ( ( ((UINT64) lwords)* ((UINT64)value_bits) )) >> 6 ) + 2 );
		}

		/** number of virtual words to machine words (floor) */
		size_t vwords_toUINT64s_lb(size_t lwords) const {
			return ( (lwords*value_bits) >> 6 );
		}

		/** bit offset of a given word */
		BYTE bitofs(size_t ofs) const {
			return (BYTE)( (ofs*value_bits) & 0x3f );
		}

		utilities::AVector<UINT64> content; ///< content array
		size_t vword_len; ///< lengths in vwords
		UINT64 value_mask; ///< mask for a single value
		bool _carry; ///< carry after addition
		bool is_slice; ///< true if this is a slice of another vector
	};

	template<BYTE value_bits>
	std::ostream & operator<< (std::ostream & o, IntegerVector<value_bits> const & mvv) {
		for(size_t j = 0; j < mvv.size(); ++j) {
			o << std::hex << std::setfill('0') << std::setw((value_bits+3)>>2) << mvv[j];
			if(value_bits > 3)
				o << "|";
		}
		o << std::dec;
		return o;
	}

	typedef IntegerVector<1> BitString;

	// TODO: smarter copy if bit counts are equal
	template<int _bpc1, int _bpc2>
	void ResizeAlphabet(IntegerVector<_bpc1> const & source, IntegerVector<_bpc2> & target) {
		//	BOOST_MPL_ASSERT_RELATION(_bpc1, <=, _bpc2);
		size_t len = source.size();
		target.resize(len);
		for(size_t j = 0; j < len; ++j) {
			target[j]= source[j];
		}
	}

};

// include partial specialisations for word lengths 8, 16 and 32
#include "IntegerVectorFixed.h"

#ifdef _USE_ASMLIB

#undef memset 
#undef memcpy 
#endif

#endif	/* _MACHINEWORD_H */

