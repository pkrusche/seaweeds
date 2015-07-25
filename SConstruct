###############################################################################
# Copyright (C) 2007   Peter Krusche, The University of Warwick
# pkrusche@gmail.com     
###############################################################################

import os
import os.path
import glob
import string
import re
import sys
import platform

from SCons.Defaults import *

import SConsHelpers.tbb
import SConsHelpers.agner_libs

###############################################################################
# read options and configure directories
###############################################################################

optfilename = 'opts.py'

# Try to find options file based on hostname
optfilename_local = 'opts_' + platform.uname()[1] + '_' + platform.uname()[0] + '_' + platform.uname()[4] + '.py'
optfilename_local_os = 'opts_' + platform.uname()[0] + '_' + platform.uname()[4] + '.py'

if os.path.exists(optfilename_local):
	optfilename = optfilename_local
elif os.path.exists(optfilename_local_os):
    optfilename = optfilename_local_os
else:
    print 'To use specific options for this platform, use options file "'+optfilename_local_os+'"'
    print 'To use specific options for this system, use options file "'+optfilename_local+'"'

print 'Using options from ' + optfilename

opts = Variables(optfilename)

# these are the options that can be specified through the command line
opts.AddVariables(
	EnumVariable('mode', 'Build mode: set to debug or release', 'debug',
                    allowed_values = ('debug', 'release', 'custom'),
                    ignorecase = 1),
	EnumVariable('simd_mode',
				 'Vector instruction set to use: mmx, sse2, or none', 'sse2',
                    allowed_values = ('mmx', 'sse2', 'nommx'),
                    ignorecase = 1),
	EnumVariable('test_verbosity',
				 'Test verbosity : none, 1, 2, 3, all', 'none',
                    allowed_values = ('none', '1', '2', '3', 'all'),
                    ignorecase = 1),
	BoolVariable('tests', 'Run tests', 0),
	BoolVariable('benchmarks', 'Run benchmarks', 0),
	BoolVariable('tuning', 'Run tuning', 0),
	BoolVariable('profile', 'Enable profiling. Also enables debug information.', 0),
	BoolVariable('debuginfo', 'Include debug information also in release version.', 1),
	BoolVariable('docs', 'Build documentation', 0),
	BoolVariable('configure', 'Perform configuration', 0),
	BoolVariable('use_yasm', 'Use yasm instead of nasm', 1),
	BoolVariable('nompi', 'Use non-mpi bsponmpi', 1),
    ('win32_boostdir', 'Path to Boost library in Win32', 'C:\\Boost\\include'),
    ('win32_ccpdir', 'Path to Microsoft Compute Cluster Pack in Win32', 'C:\\Program Files\\Microsoft Compute Cluster Pack'),
    ('win32_bsponmpidir', 'Path to bsponmpi library in Win32', 'C:\\bsponmpi'),
	('toolset', 'Specify compiler and linker tools: gcc|default', 'default'),
	('additional_lflags', 'Additional linker flags', ''),
	('additional_cflags', 'Additional compiler flags', ''),
	('MPICC', 'MPI c compiler wrapper (Unix only)', 'mpicc'),
	('MPICXX', 'MPI c++ compiler wrapper (Unix only)', 'mpicxx'),
	('MPILINK', 'MPI linker wrapper (Unix only)', 'mpicxx'),
	('replacement_CC', 'Replacement for the standard C compiler', ''),
	('replacement_CXX', 'Replacement for the standard C++ compiler', ''),
	('replacement_LIB', 'Replacement for the standard lib tool', ''),
	('replacement_LINK', 'Replacement for the standard linker ', ''),
	)

SConsHelpers.tbb.MakeOptions(opts)
SConsHelpers.agner_libs.MakeOptions(opts)

SCons.Defaults.DefaultEnvironment(tools = [])

# read options before creating root environment
readopts = Environment(tools = [], options = opts)

if int(readopts['tests']):
	print "Will run tests."
if int(readopts['benchmarks']):
	print "Will run benchmarks."

###############################################################################
# Set up the root environment
###############################################################################

# add qt4 here if necessary
platform_name = platform.uname()[0]
if platform_name == 'Windows':
	if readopts['toolset'] == "msvc":
		ttools = ['msvc', 'mslib', 'mslink', 'nasm']
	if readopts['toolset'] == 'gcc':
		ttools = ['gnulink', 'nasm', 'gcc', 'g++', 'ar']
	else:
		ttools = ['default', 'nasm']
elif platform_name == 'Linux':
	ttools = ['gnulink', 'nasm', 'gcc', 'g++', 'ar']
else:
	ttools = ['default', 'nasm']

root = Environment(
    tools = ttools,
    options = opts,
)

Help(opts.GenerateHelpText(root))

###############################################################################
# Setup compiling parameters
###############################################################################

root.Append(
	ENV = os.environ,
	BINDIR = "#bin",
	LIBDIR = "#lib",
	SRCDIR = "#src",
	CPATH = ['#src'],
	CPPPATH = ['#src'],
	)

# dependency optimization
root.SetOption('max_drift', 4)
root.SetOption('implicit_cache', 1)
root.SetOption('diskcheck', None)
root.Ignore('', '')

subarch = platform.uname()[4]
bitness = platform.architecture()[0]
platform_name = ARGUMENTS.get('OS', root['PLATFORM'])
if platform_name == 'darwin' and bitness == '64bit':
	print "MacOS X: using 64 bit mode"
	subarch = 'x86_64'
else:
	print "We are on platform " + platform_name + " " + platform.architecture()[0] + " " + subarch


mode = root['mode']
toolset = root['toolset']
profile = root['profile']
sequential = root['nompi']
debuginfo = root['debuginfo']
arch_id = str(Platform()) + '_' + toolset + '_' + mode
root.Prepend(PROGSUFFIX = "_"+arch_id)

win32_boostdir = root['win32_boostdir']
win32_ccpdir = root['win32_ccpdir']
win32_bsponmpidir = root['win32_bsponmpidir']

cxx = str(root.subst('$CXX'))
print "Compiling on platform "+ platform_name + " using mode " + mode + " and compiler " + cxx + "."

root.Append(
    CCFLAGS = ' $CFLAGS',
)

if platform_name == 'win32':
    # Win32 specific setup

    boost_include = ''
    boost_lib = ''
    # see if we can find boost
    dirs = glob.glob(win32_boostdir+"\\*")
    if len(dirs) > 0:
    	dirs.sort()
    	boost_include = dirs[len(dirs) - 1]
    	boost_lib = win32_boostdir+"\\..\\lib"

    # boost include path must go into CCFLAGS because the scons include dependency
    # parser can die otherwise
    if cxx != 'g++' and readopts['toolset'] != 'gcc':
		root.Append(
			CCFLAGS = '/I'+boost_include ,
			LIBPATH = [ boost_lib ],
		)

		if subarch == 'AMD64':
			root.Append(
				ARFLAGS = '/MACHINE:X64',
				LINKFLAGS = '/MACHINE:X64 /LTCG /LARGEADDRESSAWARE:NO',
			)
		else:
			root.Append(
				ARFLAGS = '/MACHINE:X86',
				LINKFLAGS = '/MACHINE:X86 /LARGEADDRESSAWARE:NO',
			)

    else:
		root.Append(
			CCFLAGS = ' -I'+boost_include ,
			LIBPATH = [ boost_lib ],
		)


    # see if we can find bsponmpi
    root.Append(
        CPPPATH = [ win32_bsponmpidir +'\\include' ],
        LIBPATH = [ win32_bsponmpidir +'\\lib' ],
    )
else:
		# Unix-like stuff
		root.Append(
			CCFLAGS = ' $CFLAGS',
			LIBPATH = ['.'],
		)

root.Append(
	CCFLAGS = root['additional_cflags'],
	LINKFLAGS = root['additional_lflags'],
	)

###############################################################################
# Setup debug / release mode flags
###############################################################################

if string.find(cxx, 'g++') >= 0 or string.find(cxx, 'c++') >= 0 or root['toolset'] == 'gcc':
	if mode == 'debug':
		as_debugformat = 'dwarf2'
		if subarch == 'x86_64':
			as_debugformat = 'null'
		root.Append(
		CCFLAGS=' -g -O0 -D_DEBUG',
		ASFLAGS=' -g '+as_debugformat+' ',
		)
	elif mode == 'release':
		if debuginfo:
			root.Append(
				CCFLAGS=' -g ',
				LINKFLAGS=' -g ',
			)
		if profile:
			root.Append(
				CCFLAGS=' -pg -O3',
				LINKFLAGS=' -pg',
			)
		else:
			if platform_name == 'darwin':
				root.Append(
					CCFLAGS=' -fast',
				)
			else:
				root.Append(
					CCFLAGS=' -O5',
				)
	elif mode == 'custom': # custom mode: ignore default flags
		pass

	print "using g++"
elif root['toolset'] == 'icc' and platform_name != 'win32':
	root.Append(
		CCFLAGS=' -fasm-blocks',
	)
	if mode == 'debug':
		as_debugformat = 'dwarf2'
		if subarch == 'x86_64':
			as_debugformat = 'null'
		root.Append(
		CCFLAGS=' -g -O0',
		ASFLAGS=' -g '+as_debugformat+' ',
		)
	elif mode == 'release':
		if debuginfo:
			root.Append(
				CCFLAGS=' -g ',
				LINKFLAGS=' -g ',
			)
		if profile:
			root.Append(
				CCFLAGS=' -pg -O3 -ipo',
				LINKFLAGS=' -pg',
			)
		else:
			root.Append(
			CCFLAGS=' -O3 -fast -ipo',
			)

	print "using icc on linux"
elif root['toolset'] == 'icc':
	msvc_ccflags =  " /EHsc /nologo /wd4099 /D_CRT_SECURE_NO_DEPRECATE /WL /Zi"
	if mode == 'debug':
		root.Append(
		CCFLAGS='/Debug /MDd /Od /Zi /W3 /RTC1 /RTCu /RTCs'+msvc_ccflags,
		LINKFLAGS='/DEBUG '
		)
	elif mode == 'release':
		root.Append(
			CCFLAGS='/MD /GL /O3 /Qipo /fast ' + msvc_ccflags,
		)
		if debuginfo or profile:
			root.Append(
				CCFLAGS=' /Zi ',
				LINKFLAGS=' /DEBUG '
			)
	print "using Intel C++ on Windows"
elif platform_name == 'win32':
	msvc_ccflags =  " /EHsc /nologo /wd4099 /D_CRT_SECURE_NO_DEPRECATE /WL /Zi"

	if mode == 'debug':
		root.Append(
		CCFLAGS='/Debug /MDd /Od /Zi /W3 /RTC1 /RTCu /RTCs'+msvc_ccflags,
		LINKFLAGS='/DEBUG ',
		)
	elif mode == 'release':
		root.Append(
			CCFLAGS='/MD /Ox /GL '+msvc_ccflags,
		)
		if debuginfo or profile:
			root.Append(
				CCFLAGS=' /Zi ',
				LINKFLAGS=' /DEBUG '
			)
	print "using MSVC"

if root['replacement_CC']:
	root.Replace(
		CC=root['replacement_CC']
	)
if root['replacement_CXX']:
	root.Replace(
		CXX=root['replacement_CXX']
	)
if root['replacement_LIB']:
	root.Replace(
		AR=root['replacement_LIB']
	)
if root['replacement_LINK']:
	root.Replace(
		LINK=root['replacement_LINK']
	)

###############################################################################
# Setup nasm/yasm object file types
###############################################################################

# see if we use yasm
if int(root['use_yasm']):
	root.Replace(AS = 'yasm')

if platform_name == 'win32':
	if readopts['toolset'] != 'gcc':
		if subarch == 'AMD64':
			root.Append(ASFLAGS = " -f win64 ")
		else:
			root.Append(ASFLAGS = " -f win32 ")
	else:
		if subarch == 'AMD64':
			root.Append(ASFLAGS = " -f coff ")
		else:
			root.Append(ASFLAGS = " -f coff ")

	if mode == 'debug' or profile:
		root.Append(ASFLAGS = " -g cv8")
elif platform_name == 'darwin':
	if subarch == 'x86_64':
		root.Append(ASFLAGS = " -f macho64 ")
	else:
		root.Append(ASFLAGS = " -f macho ")
else:
    if subarch == 'x86_64':
         root.Append(ASFLAGS = " -f elf64 ")
    else:
         root.Append(ASFLAGS = " -f elf ")

###############################################################################
# Setup TBB library linking
###############################################################################

SConsHelpers.tbb.MakeEnv(root)
SConsHelpers.agner_libs.MakeEnv(root)

###############################################################################
# Setup automatic parameter tuning
###############################################################################

def tuning_builder(target, source, env):
	tuning_config_h_begin = """
/*
 * This file is generated automatically and will be overwritten.
 */
#ifndef __TUNING_CONFIG_H__
#define __TUNING_CONFIG_H__

"""

	tuning_config_h_end = """

#endif /* __TUNING_CONFIG_H__ */

"""

	if int(env['tuning']):
	#	env.Command('tuning_'+arch_id+'.dat', '#bin/rangetuning', "bin/rangetuning > $TARGET")
		tuneresults = ""

		# print summary and write to file
		print "Flags added by tuning phase: \n" + tuneresults
		f = open(str(target[0]), 'w')
		f.write(tuning_config_h_begin);
		f.write(tuneresults);
		f.write(tuning_config_h_end);
	else:
		if not os.path.exists(str(target[0])):
			f = open(str(target[0]), 'w')
			f.write(tuning_config_h_begin);
			f.write(tuning_config_h_end);

if int(root['tuning']):
	bld = Builder(action = tuning_builder,
		          suffix = '.h',
			      src_suffix = '')
	root.Append(
		BUILDERS = { "Tuning" : bld }
	)
	print "Will perform tuning."

###############################################################################
# Automatic configuration code
###############################################################################

spawnp_test_cpp = """

#include <spawn.h>

#define P_WAIT 0

extern char **environ;

int main(int argc, char** argv) {
	__pid_t temp;

	posix_spawnp(&temp, "ls", NULL, NULL, NULL, environ));
	waitpid(temp, NULL, 0);
	return 0;
}

"""

def CheckSpawnp(context):
    context.Message('Checking if system supports posix spawn...')
    result = context.TryLink(spawnp_test_cpp, '.cpp')
    context.Result(result)
    return result


# configure environment
if root['configure'] != 0:
	print "Performing autoconfiguration"
	autoconfig_h_begin = """
/*
 * This file is generated automatically and will be overwritten.
 */
#ifndef __AUTOCONFIG_H__
#define __AUTOCONFIG_H__

#ifdef __cplusplus

#ifdef _MSC_VER
// Turn off global optimizations in MSVC, they FUCK THINGS UP!
//#pragma optimize("g",off)
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif
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

/** windows.h defines these as macros */
#undef min
#undef max
#endif

#else
#ifndef _WIN32
#include <sys/types.h>

typedef int32_t INT32;
typedef u_int32_t UINT32;
typedef int64_t INT64;
typedef u_int64_t UINT64;

typedef unsigned char BYTE;
typedef u_int32_t DWORD;
typedef u_int16_t WORD;
#else
#include <Windows.h>
#endif
#endif // __cplusplus

#include <bsp_config.h>

#undef ASSERT

#ifdef _DEBUG
#ifdef __GNUC__

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

inline void ASSERT_STOP(const char * f, int l) {
	void *array[10];
	size_t size;
	size = backtrace(array, 10);
	fprintf(stderr, "Error: Assertion Failed in %s:%i\\n", f, l);
	backtrace_symbols_fd(array, size, 2);
	exit(1);
}

#define ASSERT(x) do { 	\\
	if(!(x)) 			\\
		ASSERT_STOP(__FILE__, __LINE__); \\
	} while (0)

#else

#include <assert.h>
#define ASSERT assert

#endif

#else
#define ASSERT(x)
#endif

"""

	autoconfig_h_end = """

#endif /* __AUTOCONFIG_H__ */

"""
	conf = Configure(root, custom_tests = {
		'CheckSpawnp' : CheckSpawnp,
		'CheckTBB' : SConsHelpers.tbb.Check,
		'CheckAgner' : SConsHelpers.agner_libs.Check,
	})
	autohdr = open("src/autoconfig.h", 'w')
	autohdr.write(autoconfig_h_begin);

	if subarch == 'x86_64' or subarch == 'AMD64':
		autohdr.write("#define _X86_64 \n")

	if root['simd_mode'] != 'nommx':
		autohdr.write("#define _HAVE_SIMD \n")

	if root['simd_mode'] == 'sse2':
		autohdr.write("#define _HAVE_SSE2 \n")

	if conf.CheckSpawnp():
		autohdr.write("#define _POSIX_SPAWNP \n")

	if not conf.CheckTBB(3):
		print "I could not find Intel TBB version >= 3.0. Have a look at SConsHelpers/tbb.py"

	if conf.CheckAgner('asmlib'):
		print "Found Agner Fog's asmlib"
		autohdr.write("#define _USE_ASMLIB \n")	
	else:
		print "Agner Fog's asmlib was not found, will use built-in functions"

	if conf.CheckAgner('vectorclass'):
		print "Found Agner Fog's vectorclass"
		autohdr.write("#define _USE_VECTORCLASS \n")	
	else:
		print "Agner Fog's vectorclass was not found."

	autohdr.write("#define MEMCPY memcpy \n")

	autohdr.write(autoconfig_h_end)
	root = conf.Finish()
	autohdr.close()

if int(root['tuning']):
	arch_id = arch_id + "_tuned"

###############################################################################
# Setup library suffix
###############################################################################

libsuffix = ''
if sequential == 1:
	libsuffix += 'nompi'
if mode == 'debug':
	libsuffix += '_debug'

###############################################################################
# The library of assembler optimized integer operations
###############################################################################

vec_machineword_asm_suffix = root['simd_mode']
if subarch == 'AMD64':
	print "using assembler code for " + subarch
	xasmlib_files = ['src/xasmlib/machineword_AMD64.asm', 'src/xasmlib/machineword_AMD64_'+vec_machineword_asm_suffix+'.asm', 'src/xasmlib/xasmlib.c']
elif subarch == 'x86_64':
	print "using assembler code for " + subarch
	xasmlib_files = ['src/xasmlib/machineword_x86_64.asm', 'src/xasmlib/machineword_x86_64_'+vec_machineword_asm_suffix+'.asm', 'src/xasmlib/xasmlib.c']
else:
	print "using assembler code for " + subarch
	xasmlib_files = ['src/xasmlib/machineword_x86.asm', 'src/xasmlib/machineword_x86_'+vec_machineword_asm_suffix+'.asm', 'src/xasmlib/xasmlib.c']

xasmlib = root.Library('lib/xasmlib'+libsuffix, xasmlib_files)
root.Prepend(LIBS = xasmlib)

###############################################################################
# BOOST Libraries
###############################################################################

## MSVC does this automatically.
if platform_name != 'win32':
	root.Append (LIBS = ['boost_regex', 'boost_program_options', 'boost_filesystem', 'boost_system', ])

###############################################################################
# Setup MPI capable environment
###############################################################################

mpi = root.Clone()
root.Prepend(
	LIBS = [
	"bsponmpi"+libsuffix+"_mt"]
	)

if not sequential:
	if platform_name == 'win32':
		win32_ccpdir = root['win32_ccpdir']
		mpi.Append(	CPPPATH = win32_ccpdir+"\\Include",
				LIBS = ["msmpi.lib", "msmpe.lib"]
		  )
		if subarch == 'AMD64':
			mpi.Append(	LIBPATH = win32_ccpdir+"\\Lib\\amd64" )
		else:
			mpi.Append(	LIBPATH = win32_ccpdir+"\\Lib\\i386" )
	else:
		mpi.Replace(
			CXX = root['MPICXX'],
			LINK = root['MPILINK'],
			CC = root['MPICC']
		)
	mpi.Append (
		CPPDEFINES = "_HAVE_MPI",
		LIBS = ["bsponmpi"+libsuffix]
	)
else:
	mpi.Append (
		CPPDEFINES = "_SEQUENTIAL",
		LIBS = ["bsponmpi"+libsuffix]
	)

###############################################################################
# Setup automatic parameter tuning
###############################################################################

if int(root['tuning']):
	nontuned = root.Clone()
	nontuned.Append(CCFLAGS = " -D_TUNING ")

	rangetuning_objects = nontuned.Object('src/tuning/rangetuning.cpp')
	Ignore(rangetuning_objects, 'tuning_config.h')

	nontuned.Program('bin/tuning/rangetuning', rangetuning_objects)

	#root.Tuning('tuning_config.h', [
	#	'#bin/tuning/rangetuning'+dict['PROGSUFFIX'],
	#	])

###############################################################################
# Set up testing environments
###############################################################################

###############################################################################
# Export our build environments for the SConscripts
###############################################################################

Export(['root', 'mpi', 'mode', 'arch_id'])

###############################################################################
# get SConscripts
###############################################################################

## make external libraries first
SConscript('dependencies/SConscript')

test = root.Clone()
test_mpi = mpi.Clone()

test.Append (CPPDEFINES = ["TESTING"])
test_mpi.Append (CPPDEFINES = ["TESTING"])

verb = root ['test_verbosity'].lower()
defs = {
	'none' : [],
	'1' : ["_VERBOSETEST"],
	'2' : ["_VERBOSETEST", "_VERBOSETEST2"],
	'3' : ["_VERBOSETEST", "_VERBOSETEST2", "_VERBOSETEST3"],
	'all' : ["_VERBOSETEST", "_VERBOSETEST2", "_VERBOSETEST3", 
			 "_VERBOSETEST_WINDOWLCS_BOASSON", "_VERBOSETEST_WINDOWLCS_CIPR", 
			 "_DEBUG_SEAWEEDS"]
}
test.Append (CPPDEFINES = defs[verb])
test_mpi.Append (CPPDEFINES = defs[verb])


Export(['test', 'test_mpi'])

## make UnitTest++
SConscript('dependencies/UnitTest++/SConscript')
SConscript('src/SConscript')
SConscript('src/apps/SConscript')
SConscript('tests/SConscript')


