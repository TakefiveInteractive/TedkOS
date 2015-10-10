/*									tab:8
 *
 * assert.h - support for assertions and fatal exceptions
 *
 * "Copyright (c) 2005-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:	    Steve Lumetta
 * Version:	    2
 * Creation Date:   Tue Feb  1 20:09:44 2005
 * Filename:	    assert.h
 * History:
 *	SL	1	Tue Feb  1 20:09:44 2005
 *		First written.
 *	SL	2	Sun Sep 13 12:29:12 2009
 *		Adapted from later MP2 versions for use with maze game.
 */

#if !defined(ASSERT_H)
#define ASSERT_H


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*
 * This file defines several macros to simplify management of fatal 
 * exceptions, including assertions (debugging tests), aborts (dump 
 * core), and panics.
 *
 * Compiling in debug mode (NDEBUG is NOT defined) produces more 
 * informative error messages.
 *
 * The macros:
 *
 * ASSERT(cond) -- in debug mode, tests the condition and terminates the 
 *                 program if it does not hold, printing a message with file 
 *                 and line number and dumping an image of the program to
 *                 disk; does nothing if NDEBUG is defined
 * ABORT(msg) -- prints a message, dumps an image of the program to disk,
 *               and terminates the program; in debug mode, the message
 *               also includes file and line number
 * PANIC(msg) -- prints a message and terminates the program; in debug 
 *               mode, the message also includes file and line number
 *
 */
#if !defined(NDEBUG)
#define ABORT(msg)                                                       \
do {                                                                     \
    do_cleanups ();                                                      \
    fprintf (stderr, "ABORTED -- %s: %s:%d\n", msg, __FILE__, __LINE__); \
    abort ();                                                            \
} while (0);
#define ASSERT(x)                                                          \
do {                                                                       \
    if (!(x)) {                                                            \
        do_cleanups ();                                                    \
	fprintf (stderr, "ASSERTION FAILED: %s:%d\n", __FILE__, __LINE__); \
	abort ();                                                          \
    }                                                                      \
} while (0);
#define PANIC(msg)                                                     \
do {                                                                   \
    do_cleanups ();                                                    \
    fprintf (stderr, "PANIC -- %s: %s:%d\n", msg, __FILE__, __LINE__); \
    exit (3);                                                          \
} while (0);
#else /* defined(NDEBUG) */
#define ABORT(msg)                                                       \
do {                                                                     \
    do_cleanups ();                                                      \
    abort ();                                                            \
} while (0);
#define ASSERT(x)
#define PANIC(msg)                          \
do {                                        \
    do_cleanups ();                         \
    fprintf (stderr, "PANIC -- %s\n", msg); \
    exit (3);                               \
} while (0);
#endif


/*
 * We maintain a stack of functions to be called when a fatal exception 
 * occurs.  Each function takes one generic pointer argument; any
 * additional arguments must be marshaled into a block and a pointer to
 * the block passed to the function.
 *
 * Cleanups should be used in block-structured form for clarity.  For
 * example,
 *
 * push_cleanup (foo, NULL); {
 *
 *     ... some work here (note indentation!)...
 *
 * } pop_cleanup (1);
 *
 * Any functions called inside the block that generate fatal exceptions
 * then call the cleanup function (foo) before terminating the program.
 */
typedef void (*cleanup_fn_t) (void* arg);


/*
 * Request that the stack of cleanup functions be executed when certain
 * signals occur, including SIGSEGV, SIGBUS, SIGQUIT, and SIGINT.  These
 * signals are then allowed to proceed with their default behavior.
 * In addition, ignores SIGTSTP.
 */
extern void clean_on_signals ();

/* 
 * Do all cleanups on the stack; this function should not normally be 
 * called directly; instead, use the macros above (ASSERT, ABORT, or PANIC).
 */
extern void do_cleanups ();

/* 
 * Pop the top function from the stack of cleanup functions, and execute it
 * if the argument is non-zero.
 */
extern void pop_cleanup (int execute);

/* Push a cleanup function on to the stack of cleanup functions. */
extern void push_cleanup (cleanup_fn_t fn, void* arg);

#endif /* ASSERT_H */
