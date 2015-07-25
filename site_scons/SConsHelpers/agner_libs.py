import platform
import os.path
import re

###############################################################################
# Setup linking with Agner Fog's libraries (asmlib and vectorclass)
###############################################################################

###############################################################################
# Check for presence of libraries in a config context
###############################################################################

def Check(context, whichone):       
	context.Message('Checking for Agner Fog\'s %s library ' % (whichone))

	ret = 0

	if whichone == 'vectorclass':
		ret = context.TryRun("""
#include "vectorclass.h"
#include <cassert>
#include <cmath>

int main() 
{
    Vec8f a;  
    
    float q[8];					
    
    for(int i = 0; i < 8; ++i) {
    	q[i] = 0.1;
    }

    a.load(q);

    float f2 = horizontal_add(a);          // return sum of 8 elements

    assert(fabs(f2-0.8) < 0.001);

    return 0;
}
""", '.cpp')
	elif whichone == 'asmlib':
		ret = context.TryRun("""
#include "asmlib.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>

int main() 
{
	char c1[10], c2[10];
	int i = 0;

	memset(c1, 1, 10);
	memset(c2, 0, 10);

	A_memcpy(c2, c1, 10);

	for(i = 0; i < 10; ++i) {
		assert(c2[i] == 1);
	}

    return 0;
}
""" , '.cpp')
	else:
		ret = (0, 'Unknown library')
	context.Result(ret[0])	
	return ret[0]

def MakeOptions (opts):
	arch   = platform.uname()[0]
	if arch == 'Windows':
		opts.AddVariables(
	    	('veclibdir', 'Path to Agner Fog\'s vector library', '..\\vectorclass'),
	    	('asmlibdir', 'Path to Agner Fog\'s assembler library', '..\\asmlib'),
		)
	else:
		opts.AddVariables(
	    	('veclibdir', 'Path to Agner Fog\'s vector library', '../vectorclass'),
	    	('asmlibdir', 'Path to Agner Fog\'s assembler library', '../asmlib'),
		)

###############################################################################
# Add libraries to an enviroment
###############################################################################

def MakeEnv (root):
	arch   = str(platform.uname()[0]).lower()
	subarch = platform.uname()[4]
	bitness = platform.architecture()[0]
	
	if arch == 'darwin' and bitness == '64bit':
		subarch = 'x86_64'
	
	subarch = platform.uname()[4]
	veclibdir = root['veclibdir']
	asmlibdir = root['asmlibdir']

	if os.path.exists(asmlibdir):
		root.Append(
			CPPPATH = [ asmlibdir ], 
			LIBPATH = [ asmlibdir ], 
			)
		if arch == 'darwin':
			if bitness == '64bit':
				root.Append(
					LIBS = 'amac64'
				)
			else:
				root.Append(
					LIBS = 'amac32'
				)
		elif arch == 'windows':
			if bitness == '64bit':	
				root.Append(
					LIBS = 'libacof64'
				)
			else:
				root.Append(
					LIBS = 'libacof32'
				)
		elif arch == 'linux':
			if bitness == '64bit':	
				root.Append(
					LIBS = 'aelf64'
				)
			else:
				root.Append(
					LIBS = 'aelf32'
				)			
		else:
			print "FIXME: pick a library to link me with on " + arch + " " + bitness + " " + subarch

	if os.path.exists(veclibdir):
		root.Append(
			CPPPATH = [ veclibdir ], 
		)
