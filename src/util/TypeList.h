// This code was copied from the Loki library.
// 
// ////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author or Addison-Welsey Longman make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////


#ifndef TypeList_h__
#define TypeList_h__

namespace utilities {

	struct NullType {};

	template <class T, class U> struct Typelist {
		typedef T Head;
		typedef U Tail;

		typedef Typelist <T, U> type;

		template <class V>
		class Insert {
			typedef Typelist < V, Typelist <T, U > > type;
		};
	};

	typedef Typelist <NullType, NullType> EmptyTypelist;

#define TYPELIST_1(T1) ::utilities::Typelist<T1, ::utilities::NullType>

#define TYPELIST_2(T1, T2) ::utilities::Typelist<T1, TYPELIST_1(T2) >

#define TYPELIST_3(T1, T2, T3) ::utilities::Typelist<T1, TYPELIST_2(T2, T3) >

#define TYPELIST_4(T1, T2, T3, T4) \
	::utilities::Typelist<T1, TYPELIST_3(T2, T3, T4) >

#define TYPELIST_5(T1, T2, T3, T4, T5) \
	::utilities::Typelist<T1, TYPELIST_4(T2, T3, T4, T5) >

#define TYPELIST_6(T1, T2, T3, T4, T5, T6) \
	::utilities::Typelist<T1, TYPELIST_5(T2, T3, T4, T5, T6) >

#define TYPELIST_7(T1, T2, T3, T4, T5, T6, T7) \
	::utilities::Typelist<T1, TYPELIST_6(T2, T3, T4, T5, T6, T7) >

#define TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8) \
	::utilities::Typelist<T1, TYPELIST_7(T2, T3, T4, T5, T6, T7, T8) >

#define TYPELIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) \
	::utilities::Typelist<T1, TYPELIST_8(T2, T3, T4, T5, T6, T7, T8, T9) >

#define TYPELIST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) \
	::utilities::Typelist<T1, TYPELIST_9(T2, T3, T4, T5, T6, T7, T8, T9, T10) >

	template <bool flag, typename T, typename U>
	struct Select {
		typedef T Result;
	};

	template <typename T, typename U>
	struct Select<false, T, U> {
		typedef U Result;
	};


	template <typename T, typename U>
	struct IsSameType {
		enum { value = false };
	};

	template <typename T>
	struct IsSameType<T,T> {
		enum { value = true };
	};

};


#endif // TypeList_h__