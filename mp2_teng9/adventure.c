/*									tab:8
 *
 * adventure.c - main source file for ECE391 adventure game (F11 MP2)
 *
 * "Copyright (c) 2004-2011 by Steven S. Lumetta."
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
 * Version:	    3
 * Creation Date:   Fri Sep  9 21:42:12 2011
 * Filename:	    adventure.c
 * History:
 *	SL	1	Fri Sep  9 21:42:12 2011
 *		First written (based on mazegame).
 *	SL	2	Wed Sep 14 01:27:29 2011
 *		Redesigned simple game loop to manage ticks and timing.
 *	SL	3	Wed Sep 14 20:57:22 2011
 *		Cleaned up code for distribution.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "assert.h"
#include "input.h"
#include "modex.h"
#include "photo.h"
#include "text.h"
#include "world.h"


/*
 * If NDEBUG is not defined, we execute sanity checks to make sure that
 * changes to enumerations, bit maps, etc., have been made consistently.
 */
#if defined(NDEBUG)
#define sanity_check() 0
#else
static int sanity_check (void);
#endif


/* a few constants */
#define TICK_USEC      50000 /* tick length in microseconds          */
#define STATUS_MSG_LEN 40    /* maximum length of status message     */
#define MOTION_SPEED   2     /* pixels moved per command             */

/* outcome of the game */
typedef enum {GAME_WON, GAME_QUIT} game_condition_t;

/* structure used to hold game information */
typedef struct {
    room_t*      where;		 /* current room for player               */
    unsigned int map_x, map_y;   /* current upper left display pixel      */
    int          x_speed;        /* number of pixels of x motion per move */
    int          y_speed;        /* number of pixels of y motion per move */
} game_info_t;


/*
 * enumerated values, structure, and static data used for parsing typed
 * commands
 *
 * Note that the structure allows us to abbreviate commands and to create
 * synonyms for verbs (e.g., get and grab).
 */
typedef enum { /* TC = typed command */
    TC_BUY,
    TC_CHARGE,
    TC_DO,
    TC_DRINK,
    TC_DROP,
    TC_FIX,
    TC_FLASH,
    TC_GET,
    TC_GO,
    TC_INSTALL,
    TC_INVENTORY,
    TC_SIGH,
    TC_USE,
    TC_WEAR,
    NUM_TC_VALUES
} cmd_id_t;

typedef struct typed_cmd_t typed_cmd_t;
struct typed_cmd_t {
    const char* name;	/* verb that must be typed               */
    int32_t min_len;	/* minimum number of matching characters */
    cmd_id_t cmd;	/* resulting command                     */
};

static const typed_cmd_t cmd_list[] = {
    {"buy",       3, TC_BUY},
    {"charge",    2, TC_CHARGE},
    {"do",        2, TC_DO},
    {"drink",     3, TC_DRINK},
    {"drop",      2, TC_DROP},
    {"fix",       3, TC_FIX},
    {"flash",     5, TC_FLASH},
    {"get",       1, TC_GET},
    {"go",        2, TC_GO},
    {"grab",      2, TC_GET},
    {"install",   3, TC_INSTALL},
    {"inventory", 1, TC_INVENTORY},
    {"sigh",      4, TC_SIGH},
    {"use",       3, TC_USE},
    {"wear",      4, TC_WEAR},
    {NULL, 0, 0}
};


/* local functions--see function headers for details */

static void cancel_status_thread (void* ignore);
static game_condition_t game_loop (void);
static int32_t handle_typing (void);
static void init_game (void);
static void move_photo_down (void);
static void move_photo_left (void);
static void move_photo_right (void);
static void move_photo_up (void);
static void redraw_room (void);
static void* status_thread (void* ignore);
static int time_is_after (struct timeval* t1, struct timeval* t2);


/* file-scope variables */

static game_info_t game_info; /* game information */


/*
 * The variables below are used to keep track of the status message helper
 * thread, with Posix thread id recorded in status_thread_id.
 *
 * The status_msg records the current status message: when the
 * string recorded there is empty, no status message need be displayed, and
 * the status bar should instead reflect the name of the current room and the
 * player's typing (for typed commands).
 *
 * The status_msg is protected by the msg_lock mutex, which should be
 * acquired before reading or writing the message.  Further, if the message
 * is changed, the helper thread must be notified by signaling it with the
 * condition variable msg_cv (while holding the msg_lock).
 */
static pthread_t status_thread_id;
static pthread_t timer_thread_id;
static pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  msg_cv = PTHREAD_COND_INITIALIZER;
static char status_msg[STATUS_MSG_LEN + 1] = {'\0'};


/*
 * cancel_status_thread
 *   DESCRIPTION: Terminates the status message helper thread.  Used as
 *                a cleanup method to ensure proper shutdown.
 *   INPUTS: none (ignored)
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
cancel_status_thread (void* ignore)
{
    (void)pthread_cancel (status_thread_id);
}

/* Thread that bumps the seconds elapsed every second */
static
void ticker_thread (void * param)
{
    (void)(param);
    int num_seconds = 0;
    for (;;)
    {
        display_time_on_tux(num_seconds);
        num_seconds++;
        usleep(1000000);
    }
}

/*
 * cancel_ticker_thread
 *   DESCRIPTION: Terminates the ticker helper thread.  Used as
 *                a cleanup method to ensure proper shutdown.
 *   INPUTS: none (ignored)
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
cancel_ticker_thread (void* ignore)
{
    (void)pthread_cancel (timer_thread_id);
}


/*
 * game_loop
 *   DESCRIPTION: Main event loop for the adventure game.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: GAME_QUIT if the player quits, or GAME_WON if they have won
 *   SIDE EFFECTS: drives the display, etc.
 */
static game_condition_t
game_loop ()
{
    /*
     * Variables used to carry information between event loop ticks; see
     * initialization below for explanations of purpose.
     */
    struct timeval start_time, tick_time;

    struct timeval cur_time; /* current time (during tick)      */
    cmd_t cmd;               /* command issued by input control */
    int32_t enter_room;      /* player has changed rooms        */

    /* Record the starting time--assume success. */
    (void)gettimeofday (&start_time, NULL);

    /* Calculate the time at which the first event loop tick should occur. */
    tick_time = start_time;
    if ((tick_time.tv_usec += TICK_USEC) > 1000000) {
	tick_time.tv_sec++;
	tick_time.tv_usec -= 1000000;
    }


    /* The player has just entered the first room. */
    enter_room = 1;

    /* The main event loop. */
    while (1) {
	/*
	 * Update the screen, preparing the VGA palette and photo-drawing
	 * routines and drawing a new room photo first if the player has
	 * entered a new room, then showing the screen (and status bar,
	 * once you have it working).
	 */
	if (enter_room) {
	    /* Reset the view window to (0,0). */
	    game_info.map_x = game_info.map_y = 0;
	    set_view_window (game_info.map_x, game_info.map_y);

	    /* Discard any partially-typed command. */
	    reset_typed_command ();

	    /* Adjust colors and photo drawing for the current room photo. */
	    prep_room (game_info.where);

	    /* Draw the room (calls show. */
	    redraw_room ();

	    /* Only draw once on entry. */
	    enter_room = 0;
	}

	show_screen ();

    unsigned char buf[STATUS_BAR_OFFSET * 4];
    memset(buf, 10, sizeof(buf));       // cyan

    pthread_mutex_lock(&msg_lock);      // Enter critical section
    if (status_msg[0] == '\0')          // If we have status message, display that instead
    {
        draw_typed_command(buf, get_typed_command());
        draw_room_name(buf, room_name(game_info.where));
    } else {
        draw_status_msg(buf, status_msg);
    }
    pthread_mutex_unlock(&msg_lock);    // Leave critical section

    /* Draw content onto screen */
    show_status_bar(buf);

	/*
	 * Wait for tick.  The tick defines the basic timing of our
	 * event loop, and is the minimum amount of time between events.
	 */
	do {
	    if (gettimeofday (&cur_time, NULL) != 0) {
		/* Panic!  (should never happen) */
		clear_mode_X ();
		shutdown_input ();
		perror ("gettimeofday");
		exit (3);
	    }
	} while (!time_is_after (&cur_time, &tick_time));

	/*
	 * Advance the tick time.  If we missed one or more ticks completely,
	 * i.e., if the current time is already after the time for the next
	 * tick, just skip the extra ticks and advance the clock to the one
	 * that we haven't missed.
	 */
	do {
	    if ((tick_time.tv_usec += TICK_USEC) > 1000000) {
		tick_time.tv_sec++;
		tick_time.tv_usec -= 1000000;
	    }
	} while (time_is_after (&cur_time, &tick_time));

	/*
	 * Handle asynchronous events.  These events use real time rather
	 * than tick counts for timing, although the real time is rounded
	 * off to the nearest tick by definition.
	 */
	/* (none right now...) */

	/*
	 * Handle synchronous events--in this case, only player commands.
	 * Note that typed commands that move objects may cause the room
	 * to be redrawn.
	 */

	cmd = get_command ();
	switch (cmd) {
	    case CMD_UP:    move_photo_down ();  break;
	    case CMD_RIGHT: move_photo_left ();  break;
	    case CMD_DOWN:  move_photo_up ();    break;
	    case CMD_LEFT:  move_photo_right (); break;
	    case CMD_MOVE_LEFT:
		enter_room = (TC_CHANGE_ROOM ==
			      try_to_move_left (&game_info.where));
		break;
	    case CMD_ENTER:
		enter_room = (TC_CHANGE_ROOM ==
			      try_to_enter (&game_info.where));
		break;
	    case CMD_MOVE_RIGHT:
		enter_room = (TC_CHANGE_ROOM ==
			      try_to_move_right (&game_info.where));
		break;
	    case CMD_TYPED:
		if (handle_typing ()) {
		    enter_room = 1;
		}
		break;
	    case CMD_QUIT: return GAME_QUIT;
	    default: break;
	}

	/* If player wins the game, their room becomes NULL. */
	if (NULL == game_info.where) {
	    return GAME_WON;
	}
    } /* end of the main event loop */
}


/*
 * handle_typing
 *   DESCRIPTION: Parse and execute a typed command.
 *   INPUTS: none (reads typed command)
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if the player's room changes, 0 otherwise
 *   SIDE EFFECTS: may move the player, move objects, and/or redraw the screen
 */
static int32_t
handle_typing ()
{
    const char*      cmd;     /* command verb typed                */
    int32_t          cmd_len; /* length of command verb            */
    const char*      arg;     /* argument given to command verb    */
    int32_t          idx;     /* loop index over command list      */
    tc_action_t      result;  /* result of typed command execution */

    /* Read the command and strip leading spaces.  If it's empty, return. */
    cmd = get_typed_command ();
    while (' ' == *cmd) { cmd++; }
    if ('\0' == *cmd) { return 0; }

    /*
     * Walk over the command verb, calculating its length as we go.  Space
     * or NUL marks the end of the verb, after which the argument begins.
     * Leading spaces are first stripped from the argument, but we make no
     * attempt to deal with trailing spaces (argument names must match
     * exactly).
     */
    for (cmd_len = 0; ' ' != cmd[cmd_len] && '\0' != cmd[cmd_len]; cmd_len++);
    arg = &cmd[cmd_len];
    while (' ' == *arg) { arg++; }

    /* Compare the typed verb with each command in our list. */
    for (idx = 0; NULL != cmd_list[idx].name; idx++) {

        /* If the typed verb is not long enough, it can't match. */
	if (cmd_list[idx].min_len > cmd_len) { continue; }

	/* Compare the prefix of the command with the typed verb. */
        if (0 != strncasecmp (cmd_list[idx].name, cmd, cmd_len)) { continue; }

	/* Execute the command found. */
	switch (cmd_list[idx].cmd) {
	    case TC_BUY:
	        result = typed_cmd_buy (&game_info.where, arg);
		break;
	    case TC_CHARGE:
	        result = typed_cmd_charge (&game_info.where, arg);
		break;
	    case TC_DO:
	        result = typed_cmd_do (&game_info.where, arg);
		break;
	    case TC_DRINK:
	        result = typed_cmd_drink (&game_info.where, arg);
		break;
	    case TC_DROP:
	        result = typed_cmd_drop (&game_info.where, arg);
		if (!player_has_board ()) {
		    game_info.x_speed = MOTION_SPEED;
		}
		if (!player_has_jetpack ()) {
		    game_info.y_speed = MOTION_SPEED;
		}
		break;
	    case TC_FIX:
	        result = typed_cmd_fix (&game_info.where, arg);
		break;
	    case TC_FLASH:
	        result = typed_cmd_flash (&game_info.where, arg);
		break;
	    case TC_GET:
	        result = typed_cmd_get (&game_info.where, arg);
		if (player_has_board ()) {
		    game_info.x_speed = MOTION_SPEED * 3;
		}
		if (player_has_jetpack ()) {
		    game_info.y_speed = MOTION_SPEED * 3;
		}
		break;
	    case TC_GO:
	        result = typed_cmd_go (&game_info.where, arg);
		break;
	    case TC_INSTALL:
	        result = typed_cmd_install (&game_info.where, arg);
		break;
	    case TC_INVENTORY:
	        result = typed_cmd_inventory (&game_info.where, arg);
		break;
	    case TC_SIGH:
	        result = typed_cmd_sigh (&game_info.where, arg);
		break;
	    case TC_USE:
	        result = typed_cmd_use (&game_info.where, arg);
		break;
	    case TC_WEAR:
	        result = typed_cmd_wear (&game_info.where, arg);
		break;
	    default:
		show_status ("Bug...!");
		result = TC_ALLOW_EDIT;
	        break;
	}

	/* Handle command result and return. */
	if (TC_CHANGE_ROOM == result) {
	    return 1;
	}
	if (TC_ALLOW_EDIT != result) {
	    reset_typed_command ();
	    if (TC_REDRAW_ROOM == result) {
	        redraw_room ();
	    }
	}
	return 0;
    }

    /* The command was not recognized. */
    show_status ("What are you babbling about?");
    return 0;
}


/*
 * init_game
 *   DESCRIPTION: Initialize the game information, including the initial
 *                room, photo display, motion speed, and so forth.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
init_game ()
{
    game_info.where = start_in_room ();
    game_info.map_x = 0;
    game_info.map_y = 0;
    game_info.x_speed = MOTION_SPEED;
    game_info.y_speed = MOTION_SPEED;
}


/*
 * move_photo_down
 *   DESCRIPTION: Move background photo down one or more pixels.  Amount of
 *                motion depends on game_info.y_speed.  Movement stops at
 *                upper edge of photo.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: shifts view window
 */
static void
move_photo_down ()
{
    int32_t delta; /* Number of pixels by which to move. */
    int32_t idx;   /* Index over rows to redraw.         */

    /* Calculate the number of pixels by which to move. */
    delta = (game_info.y_speed > game_info.map_y ?
	     game_info.map_y : game_info.y_speed);

    /* Shift the logical view upward. */
    game_info.map_y -= delta;
    set_view_window (game_info.map_x, game_info.map_y);

    /* Draw the newly exposed lines. */
    for (idx = 0; delta > idx; idx++) {
	(void)draw_horiz_line (idx);
    }
}


/*
 * move_photo_left
 *   DESCRIPTION: Move background photo left one or more pixels.  Amount of
 *                motion depends on game_info.x_speed.  Movement stops at
 *                right edge of photo.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: shifts view window
 */
static void
move_photo_left ()
{
    int32_t delta; /* Number of pixels by which to move. */
    int32_t idx;   /* Index over columns to redraw.      */

    /* Calculate the number of pixels by which to move. */
    delta = room_photo_width (game_info.where) - SCROLL_X_DIM -
    	    game_info.map_x;
    delta = (game_info.x_speed > delta ? delta : game_info.x_speed);

    /* Shift the logical view to the right. */
    game_info.map_x += delta;
    set_view_window (game_info.map_x, game_info.map_y);

    /* Draw the newly exposed lines. */
    for (idx = 1; delta >= idx; idx++) {
	(void)draw_vert_line (SCROLL_X_DIM - idx);
    }
}


/*
 * move_photo_right
 *   DESCRIPTION: Move background photo right one or more pixels.  Amount of
 *                motion depends on game_info.x_speed.  Movement stops at
 *                left edge of photo.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: shifts view window
 */
static void
move_photo_right ()
{
    int32_t delta; /* Number of pixels by which to move. */
    int32_t idx;   /* Index over columns to redraw.      */

    /* Calculate the number of pixels by which to move. */
    delta = (game_info.x_speed > game_info.map_x ?
	     game_info.map_x : game_info.x_speed);

    /* Shift the logical view to the left. */
    game_info.map_x -= delta;
    set_view_window (game_info.map_x, game_info.map_y);

    /* Draw the newly exposed lines. */
    for (idx = 0; delta > idx; idx++) {
	(void)draw_vert_line (idx);
    }
}


/*
 * move_photo_up
 *   DESCRIPTION: Move background photo up one or more pixels.  Amount of
 *                motion depends on game_info.y_speed.  Movement stops at
 *                lower edge of photo.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: shifts view window
 */
static void
move_photo_up ()
{
    int32_t delta; /* Number of pixels by which to move. */
    int32_t idx;   /* Index over rows to redraw.         */

    /* Calculate the number of pixels by which to move. */
    delta = room_photo_height (game_info.where) - SCROLL_Y_DIM -
    	    game_info.map_y;
    delta = (game_info.y_speed > delta ? delta : game_info.y_speed);

    /* Shift the logical view upward. */
    game_info.map_y += delta;
    set_view_window (game_info.map_x, game_info.map_y);

    /* Draw the newly exposed lines. */
    for (idx = 1; delta >= idx; idx++) {
	(void)draw_horiz_line (SCROLL_Y_DIM - idx);
    }
}


/*
 * redraw_room
 *   DESCRIPTION: Draw all lines on the screen.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Draws the entire screen (but not the status bar).
 */
static void
redraw_room ()
{
    int32_t i; /* index over rows */

    /* Draw all lines in the scroll region. */
    for (i = 0; i < SCROLL_Y_DIM; i++) {
	(void)draw_horiz_line (i);
    }
}


/*
 * status_thread
 *   DESCRIPTION: Function executed by status message helper thread.
 *                Waits for a message to be displayed, then shows the
 *                message for 1.5 seconds before deleting it.  If a
 *                new message has appeared in the meantime, restarts the
 *                clock and tries again.
 *   INPUTS: none (ignored)
 *   OUTPUTS: none
 *   RETURN VALUE: NULL
 *   SIDE EFFECTS: Changes the status message to an empty string.
 */
static void*
status_thread (void* ignore)
{
    struct timespec ts; /* absolute wake-up time */

    while (1) {

	/*
	 * Wait for a message to appear.  Note that we must check the
	 * condition after acquiring the lock, and that pthread_cond_wait
	 * yields the lock, then reacquires the lock before returning.
	 */
	(void)pthread_mutex_lock (&msg_lock);
	while ('\0' == status_msg[0]) {
	    pthread_cond_wait (&msg_cv, &msg_lock);
	}

	/*
	 * A message is present: if we stop before the timeout
	 * passes, assume that a new one has been posted; if the
	 * timeout passes, clear the message and wait for a new one.
	 */
	do {
	    /* Get the current time. */
	    clock_gettime (CLOCK_REALTIME, &ts);

	    /* Add 1.5 seconds to it. */
	    if (500000000 <= ts.tv_nsec) {
		ts.tv_sec += 2;
		ts.tv_nsec -= 500000000;
	    } else {
		ts.tv_sec += 1;
		ts.tv_nsec += 500000000;
	    }

	    /*
	     * And go to sleep.  If we wake up due to anything but a
	     * timeout, we assume (possibly incorrectly) that a new
	     * message has appeared and try to wait 1.5 seconds again.
	     */
	} while (ETIMEDOUT !=
		 pthread_cond_timedwait (&msg_cv, &msg_lock, &ts));

	/*
	 * Clear the message, then release the lock (remember that
	 * pthread_cond_timedwait reacquires the lock before returning).
	 */
	status_msg[0] = '\0';
	(void)pthread_mutex_unlock (&msg_lock);
    }

    /* This code never executes--the thread should always be cancelled. */
    return NULL;
}


/*
 * time_is_after
 *   DESCRIPTION: Check whether one time is at or after a second time.
 *   INPUTS: t1 -- the first time
 *           t2 -- the second time
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if t1 >= t2
 *                 0 if t1 < t2
 *   SIDE EFFECTS: none
 */
static int
time_is_after (struct timeval* t1, struct timeval* t2)
{
    if (t1->tv_sec == t2->tv_sec)
        return (t1->tv_usec >= t2->tv_usec);
    if (t1->tv_sec > t2->tv_sec)
        return 1;
    return 0;
}


/*
 * show_status (interface function; declared in world.h)
 *   DESCRIPTION: Show a specific status message of up to STATUS_MSG_LEN
 *                characters.
 *   INPUTS: s -- the string used for the status message
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Overwrites any previous message.
 */
void
show_status (const char* s)
{
    /* msg_lock critical section starts here. */
    (void)pthread_mutex_lock (&msg_lock);

    /* Copy the new message under the protection of msg_lock. */
    strncpy (status_msg, s, STATUS_MSG_LEN);
    status_msg[STATUS_MSG_LEN] = '\0';

    /*
     * Wake up the status message helper thread.  Note that we still hold
     * the msg_lock, so the thread can't wake up until we release it.
     */
    (void)pthread_cond_signal (&msg_cv);

    /* msg_lock critical section ends here. */
    (void)pthread_mutex_unlock (&msg_lock);
}


/*
 * main
 *   DESCRIPTION: Play the adventure game.
 *   INPUTS: none (command line arguments are ignored)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, 3 in panic situations
 */
int
main ()
{
    game_condition_t game;  /* outcome of playing */

    /* Randomize for more fun (remove for deterministic layout). */
    srand (time (NULL));

    /* Provide some protection against fatal errors. */
    clean_on_signals ();

    if (!build_world ()) {PANIC ("can't build world");}
    init_game ();

    /* Perform sanity checks. */
    if (0 != sanity_check ()) {
        PANIC ("failed sanity checks");
    }

    /* Create status message thread. */
    if (0 != pthread_create (&status_thread_id, NULL, status_thread, NULL)) {
        PANIC ("failed to create status thread");
    }
    push_cleanup (cancel_status_thread, NULL); {

        /* Start mode X. */
        if (0 != set_mode_X (fill_horiz_buffer, fill_vert_buffer)) {
            PANIC ("cannot initialize mode X");
        }
        push_cleanup ((cleanup_fn_t)clear_mode_X, NULL); {

            /* Initialize the keyboard and/or Tux controller. */
            if (0 != init_input ()) {
                PANIC ("cannot initialize input");
            }

            push_cleanup ((cleanup_fn_t)shutdown_input, NULL); {

                if (0 != pthread_create(&timer_thread_id, NULL, (void*)ticker_thread, NULL)) {
                    PANIC ("failed to create ticker thread");
                }

                push_cleanup (cancel_ticker_thread, NULL); {

                    game = game_loop ();

                } pop_cleanup (1);

            } pop_cleanup (1);

        } pop_cleanup (1);

    } pop_cleanup (1);

    /* Print a message about the outcome. */
    switch (game) {
        case GAME_WON: printf ("You win the game!  CONGRATULATIONS!\n"); break;
        case GAME_QUIT: printf ("Quitter!\n"); break;
    }

    /* Return success. */
    return 0;
}


#if !defined(NDEBUG)
/*
 * sanity_check
 *   DESCRIPTION: Perform checks on changes to constants and enumerated values.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if checks pass, -1 if any fail
 *   SIDE EFFECTS: none
 */
static int
sanity_check ()
{
    int32_t cnt[NUM_TC_VALUES]; /* count of synonymous commands      */
    int32_t idx;                /* index over list of typed commands */
    int32_t ret_val;            /* return value                      */

    /* Initialize return value. */
    ret_val = 0;

    /* Check typed command list. */
    (void)memset (cnt, 0, sizeof (cnt));
    for (idx = 0; NULL != cmd_list[idx].name; idx++) {
	if (1 > cmd_list[idx].min_len) {
	    fprintf (stderr, "Typed command %s always matches.\n",
		     cmd_list[idx].name);
	    ret_val = -1;
	    continue;
	}
	if (cmd_list[idx].min_len > strlen (cmd_list[idx].name)) {
	    fprintf (stderr, "Typed command %s can never match.\n",
		     cmd_list[idx].name);
	    ret_val = -1;
	    continue;
	}
        if (0 > cmd_list[idx].cmd || NUM_TC_VALUES <= cmd_list[idx].cmd) {
	    fprintf (stderr, "Typed command %s has invalid command number.\n",
		     cmd_list[idx].name);
	    ret_val = -1;
	    continue;
	}
	cnt[cmd_list[idx].cmd]++;
    }

    /*
     * Now check that every typed command can be issued with some string.
     * We could be fancier and check that it's possible to match (shadowing
     * can prevent it: matching "a" in entry #1 prevents matching "an" in
     * entry #2.).
     */
    for (idx = 0; NUM_TC_VALUES > idx; idx++) {
        if (0 == cnt[idx]) {
	    fprintf (stderr, "TC_ #%d has no valid command strings.\n", idx);
	    ret_val = -1;
	}
    }

    /* Return success/failure. */
    return ret_val;
}
#endif /* !defined(NDEBUG) */
