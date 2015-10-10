/*									tab:8
 *
 * world.h - world content header file for the ECE391 MP2 F11 adventure game
 *
 * "Copyright (c) 2011 by Steven S. Lumetta."
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
 * Version:	    1
 * Creation Date:   Tue Sep 13 23:47:11 2011
 * Filename:	    world.h
 * History:
 *	SL	1	Tue Sep 13 23:47:11 2011
 *		Completed initial implementation.
 *	SL	2	Wed Sep 14 23:17:14 2011
 *		Cleaned up code for distribution.
 */
#if !defined(WORLD_H)
#define WORLD_H


#include "types.h"


/* structure access functions */
extern uint16_t obj_get_x (const object_t* obj);
extern uint16_t obj_get_y (const object_t* obj);
extern image_t* obj_image (const object_t* obj);
extern object_t* obj_next (const object_t* obj);
extern object_t* room_contents_iterate (const room_t* r);
extern const char* room_name (const room_t* r);
extern photo_t* room_photo (const room_t* r);
extern uint32_t room_photo_height (const room_t* r);
extern uint32_t room_photo_width (const room_t* r);

/* Build the game world.  Returns 0 on failure, or 1 on success. */
extern int32_t build_world (void);

/* Get pointer to starting room for player. */
extern room_t* start_in_room (void);

/*
 * checks for accelerator object ownership; these make horizontal (board)
 * and vertical (jetpack) pixel panning faster
 */
extern int32_t player_has_board ();
extern int32_t player_has_jetpack ();

/* responses possible for command actions */
typedef enum {
    TC_ALLOW_EDIT,   /* allow user to edit current text         */
    TC_DISCARD_TEXT, /* reset the typed text, but don't redraw  */
    TC_REDRAW_ROOM,  /* objects may have moved--redraw the room */
    TC_CHANGE_ROOM   /* room changed--update view, etc.         */
} tc_action_t;

/* actions caused by button presses (room movement) */
extern tc_action_t try_to_move_left (room_t** rptr);
extern tc_action_t try_to_enter (room_t** rptr);
extern tc_action_t try_to_move_right (room_t** rptr);

/* typed command actions */
extern tc_action_t typed_cmd_buy (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_charge (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_do (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_drink (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_drop (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_fix (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_flash (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_get (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_go (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_install (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_inventory (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_sigh (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_use (room_t** rptr, const char* arg);
extern tc_action_t typed_cmd_wear (room_t** rptr, const char* arg);

/* in adventure.c */
extern void show_status (const char* s);

#endif /* WORLD_H */
