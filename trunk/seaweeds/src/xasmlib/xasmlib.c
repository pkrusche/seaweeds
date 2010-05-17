/***************************************************************************
*   Copyright (C) 2009 by Peter Krusche                                   *
*   peter@dcs.warwick.ac.uk                                               *
***************************************************************************/

#include <stdlib.h>

#ifndef _WIN32
#define __cdecl
#endif


extern void __cdecl do_emms();
extern void __cdecl initbitmasks();

void exit_xasmlib(void) {
#ifndef _NO_MMX
	do_emms();
#endif
}

void init_xasmlib() {
	initbitmasks();
	atexit(exit_xasmlib);
}

