## SCons Brook+ Builder 
#
# Peter Krusche, 2009
# 
import os.path

from SCons.Action import *
from SCons.Builder import *

def exists(env):
	"""
	Make sure doxygen exists.
	"""
	try: 
		br = env['BROOKROOT']
		if not (br == ''):
			return br
	except KeyError: pass

	try: return os.environ['BROOKROOT']
	except KeyError: pass

	return None

def locateCommand(env, command, brookroot) :
	suffixes = [
		'',
		'.exe',
	]
	triedPaths = []
	for suffix in suffixes :
		fullpath = os.path.join(brookroot, command + suffix)
		if os.access(fullpath, os.X_OK) :
			return fullpath
		triedPaths.append(fullpath)
		fullpath = env.Detect(command)
		if not (fullpath is None): 
			return fullpath

	raise Exception("Brook+ compiler '" + command + "' not found. Tried: " + ', '.join(triedPaths))
   
def generate(env):
	env.Replace(
		BROOKROOT  = exists(env),
		BINPATH = os.path.join(env['BROOKROOT'], 'sdk', 'bin'),
		BRCC = locateCommand(env, 'brcc', os.path.join(env['BROOKROOT'], 'sdk', 'bin')),
		BRCCFLAGS = '',
		BROOK_CPPPATH = os.path.join(env['BROOKROOT'], 'sdk', 'include'),
		BROOK_LIBPATH = os.path.join(env['BROOKROOT'], 'sdk', 'lib'),
	)
	
	brccbuilder = Builder(
		action = SCons.Action.Action('"$BRCC" $BRCCFLAGS $SOURCES'),
		src_suffix = '.br',
		suffix = '.cpp',
		single_source = True
	)

	env.Append( BUILDERS = { 'BrCC' : brccbuilder } )
