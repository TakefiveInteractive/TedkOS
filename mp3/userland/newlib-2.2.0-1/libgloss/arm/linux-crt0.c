/** Linux startup code for the ARM processor.
 * Written by Shaun Jackman <sjackman@gmail.com>.
 * Copyright 2006 Pathway Connectivity
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include <stdlib.h>
#include <unistd.h>
#include "arm.h"

static int _main(int argc, char *argv[]) __attribute__((noreturn));

#if defined(__thumb__) && !defined(THUMB_V7_V6M)
asm("\n"
	".code 32\n"
	".global _start\n"
	".type _start, %function\n"
	"_start:\n"
	"\tldr r0, .LC0\n"
	"\tbx r0\n"
	".LC0:\n"
	"\t.word _start_thumb\n"
	".size _start, .-_start\n");

__attribute__((naked, used))
static void _start_thumb(void)
#else
__attribute__((naked))
void _start(void)
#endif
{
	register int *sp asm("sp");
	_main(*sp, (char **)(sp + 1));
}

static int _main(int argc, char *argv[])
{
	environ = argv + argc + 1;
	exit(main(argc, argv, environ));
}
