/*									tab:8
 *
 * assert.c - support for assertions and fatal exceptions
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
 * Creation Date:   Tue Feb  1 20:26:56 2005
 * Filename:	    assert.c
 * History:
 *	SL	1	Tue Feb  1 20:26:56 2005
 *		First written.
 *	SL	2	Sun Sep 13 12:30:05 2009
 *		Adapted from later MP2 versions for use with maze game.
 */


#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert.h"


/*
 * The cleanup stack is implemented as a linked list of cleanup_t structures,
 * defined here.
 */
typedef struct cleanup_t cleanup_t;
struct cleanup_t {
    cleanup_fn_t fn;    /* the function to be called                  */
    void*        arg;   /* the argument to pass to the function       */
    cleanup_t*   next;  /* a pointer to the next cleanup on the stack */
};


/* Helper function prototypes; see function headers for further information. */
static void catch_signal (int sig);
static void set_signal_behavior (int32_t num);


/*
 * MODULE VARIABLES
 */

/* This variable points to the top of the cleanup stack. */
static cleanup_t* cleanup_stack = NULL;



/*
 * INTERFACE FUNCTIONS -- these functions serve as entry points from other
 * modules
 */


/* 
 * clean_on_signals
 *   DESCRIPTION: Request that the stack of cleanup functions be executed 
 *                when certain signals occur, including SIGSEGV, SIGBUS, 
 *                SIGQUIT, and SIGINT.  These signals are then allowed to 
 *                proceed with their default behavior.  In addition, 
 *                ignores SIGTSTP.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: discards (most of) previous signal behavior specifications
 *   WARNING: not compatible with other redefinitions of behavior for the
 *            specified signals
 */
void
clean_on_signals ()
{
    struct sigaction sa;   /* signal behavior definition structure  */

    /* Use helper function to set up behavior for these four signals. */
    set_signal_behavior (SIGINT);
    set_signal_behavior (SIGSEGV);
    set_signal_behavior (SIGBUS);
    set_signal_behavior (SIGQUIT);

    /* Ignore stop requests from the keyboard (tty). */
    sa.sa_handler = SIG_IGN;
    if (-1 == sigaction (SIGTSTP, &sa, NULL))
        PANIC ("writing signal action failed");
}


/* 
 * do_cleanups
 *   DESCRIPTION: Do all cleanups on the stack.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: executes all cleanups and frees alls structure use to 
 *                 store cleanup function information
 */
void 
do_cleanups ()
{
    /* Pop until the stack is empty. */
    while (NULL != cleanup_stack)
        pop_cleanup (1);
}


/* 
 * pop_cleanup
 *   DESCRIPTION: Pop the top function from the stack of cleanup 
 *                functions, and execute it if the argument is non-zero.
 *   INPUTS: execute -- popped cleanup function is executed if non-zero
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: frees the structure use to store cleanup function 
 *                 information
 */
void 
pop_cleanup (int execute)
{
    cleanup_t* c;

    /* Module interface: check for stack underflow. */
    ASSERT (NULL != cleanup_stack);

    /* Check underflow again, in case it escaped our debugging runs. */
    if (NULL != (c = cleanup_stack)) {
	cleanup_stack = c->next; /* Remove top element from stack. */
	if (execute)             /* Execute it if requested.       */
	    (*(c->fn)) (c->arg);
	free (c);                /* And free the structure.        */
    }
}


/* 
 * push_cleanup
 *   DESCRIPTION: Push a cleanup function on to the stack of cleanup 
 *                functions.
 *   INPUTS: fn -- the cleanup function to be pushed
 *           arg -- the argument to the cleanup function (when called)
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: dynamically allocates a structure to store cleanup
 *                 function information
 */
void 
push_cleanup (cleanup_fn_t fn, void* arg)
{
    cleanup_t* c;    /* new cleanup stack element */

    /* Module interface: check argument validity. */
    ASSERT (NULL != fn);

    if (NULL == (c = malloc (sizeof (*c)))) /* Create a new structure.     */
        ABORT ("out of memory");
    c->fn         = fn;                     /* Fill in the new structure.  */
    c->arg        = arg;
    c->next       = cleanup_stack;          /* And push it onto the stack. */
    cleanup_stack = c;
}



/*
 * HELPER FUNCTIONS -- these functions are only called from other functions
 * in this file
 */


/* 
 * catch_signal
 *   DESCRIPTION: Signal handler used to execute cleanups before termination.
 *   INPUTS: sig -- signal number that invoked handler
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: executes stack of cleanup functions and regenerates
 *                 signal to force default signal behavior
 *   WARNING: will loop forever if signal's default behavior is not restored
 *            after handler execution!
 */
static void
catch_signal (int sig)
{
    do_cleanups ();
    kill (getpid (), sig);
}


/* 
 * set_signal_behavior
 *   DESCRIPTION: Installs a signal handler that executes the cleanup stack
 *                and requests that the default signal behavior be restored 
 *                after execution of the handler.
 *   INPUTS: num -- signal number for which behavior should be changed
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: discards (most of) previous signal behavior specification
 *   WARNING: not compatible with other redefinitions of behavior for the
 *            specified signal
 */
static void
set_signal_behavior (int32_t num)
{
    struct sigaction sa;   /* signal behavior definition structure  */

    /* Read current signal behavior. */
    if (-1 == sigaction (num, NULL, &sa))
        PANIC ("reading signal action failed");

    /*
     * Set signal handler and request restoration to default behavior of
     * signal after handler executes.
     */
    sa.sa_handler = catch_signal;
    sa.sa_flags |= SA_ONESHOT;

    /* Install new behavior for signal. */
    if (-1 == sigaction (num, &sa, NULL))
        PANIC ("writing signal action failed");
}

