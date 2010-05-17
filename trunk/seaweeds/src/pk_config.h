/***************************************************************************
 *   Copyright (C) 2007   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/

#ifndef __PKCONFIG_H_
#define __PKCONFIG_H_

#ifdef _MSC_VER
// Turn off global optimizations, they FUCK THINGS UP!
#pragma optimize("g",off)

#else
#define __cdecl
#endif


#ifndef _WIN32
#include <typeinfo>

#include <boost/cstdint.hpp>

typedef boost::int32_t INT32;
typedef boost::uint32_t UINT32;
typedef boost::int64_t INT64;
typedef boost::uint64_t UINT64;

typedef boost::uint8_t BYTE;
typedef boost::uint32_t DWORD;
typedef boost::uint16_t WORD;
#else
#include <Windows.h>

#endif


#ifndef _TUNING
#include "../tuning_config.h"
#endif

#ifdef _DEBUG
#include <assert.h>

#define ASSERT assert
#else

#define ASSERT(x)

#endif
#define STD_MEMCPY
#ifdef STD_MEMCPY
#define MEMCPY memcpy
#else
#include "../libs/asmlib/asmlib.h"
#define MEMCPY A_memcpy
#endif

#include "../autoconfig.h"

#endif /*CONFIG_H_*/
