
import os.path
import platform;

import SCons.Action
import SCons.Builder
import SCons.Defaults
import SCons.Scanner
import SCons.Tool
import SCons.Util

compilers = ["cilkpp", "cilk++" ]

def exists(env):
	return env.Detect(compilers)

def generate(env):
	import SCons.Tool
	import SCons.Tool.cc
	static_obj, shared_obj = SCons.Tool.createObjBuilders(env)

	suffix = ".cilk"
	static_obj.add_action(suffix, SCons.Action.Action('$CILKPPCOM'))
	static_obj.add_emitter(suffix, SCons.Defaults.StaticObjectEmitter)

	if platform.uname()[0] == 'Windows':
		cilkobjout = ' /c /Fo'
		env['CILKPP'] = 'cilkpp'
	else:
		cilkobjout = ' -c -o '
		env['CILKPP'] = 'cilk++'

	env['CILKFLAGS']   = SCons.Util.CLVar('')
	env['CILKPPCOM'] = '$CILKPP $CILKFLAGS $CXXFLAGS $CCFLAGS $_CCCOMCOM $SOURCE ' + cilkobjout + '$TARGET' 

