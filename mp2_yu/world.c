/*									tab:8
 *
 * world.c - world content source file for the ECE391 MP2 F11 adventure game
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
 * Creation Date:   Wed Sep 14 02:19:41 2011
 * Filename:	    world.c
 * History:
 *	SL	1	Wed Sep 14 02:19:41 2011
 *		Completed initial implementation.
 *	SL	2	Thu Sep 15 00:31:40 2011
 *		Cleaned up code for distribution.
 */
 

#include <string.h>
#include <strings.h>

#include "assert.h"
#include "photo.h"
#include "world.h"


/* parameters defined for this file */

/* room identifiers */
enum {
    R_NONE = -1,

    /* Area 0: The Backpack */
    R_INVENTORY,

    /* Area 1: Everitt and Green Street */
    R_IN_391LAB,	/* inside the 391 lab               */
    R_BY_391LAB,	/* outside of the 391 lab           */
    R_IN_IEEE,		/* inside the IEEE/HKN office       */
    R_BY_IEEE,		/* outside of the IEEE/HKN office   */
    R_IN_395LAB,	/* inside the 395 lab               */
    R_BY_395LAB,	/* outside of the 395 lab           */
    R_EVT_STAIR,	/* Everitt Lab's eastern stairwell  */
    R_IN_CLEANR,	/* inside the cleanroom             */
    R_BY_CLEANR,	/* outside of the cleanroom         */
    R_EVRT_VEND,	/* near the Everitt vending machine */
    R_ALMAMATER,	/* near the Alma Mater statue       */
    R_IN_COCOMR,	/* inside of Cocomero               */
    R_BY_COCOMR,	/* just outside of Cocomero         */
    R_BY_ZAS,		/* across from the ruins of Za's    */
    R_EAST_EVRT,	/* East entrance of Everitt Lab     */
    R_EVRT_BSMT,	/* entrance to Everitt Lab basement */

    /* Area 2: Bardeen Quad and Environs */
    R_WEST_BONE,	/* looking West along the Boneyard   */
    R_CIRCLE_N,		/* Boneyard Bridge looking North     */
    R_CIRCLE_SW,	/* Boneyard Bridge looking Southwest */
    R_EAST_BONE,	/* looking East along the Boneyard   */
    R_BARDEEN,		/* Bardeen Quad                      */
    R_LIB_BACK,		/* rear of Grainger library          */
    R_RESERVE,		/* Grainger reserve desk             */
    R_TALBOT_NW,	/* looking Northwest at Talbot       */
    R_TALBOT_SW,	/* looking Southwest at Talbot       */
    R_TALBOT,		/* inside Talbot Laboratory          */
    R_SPRINGFLD,	/* looking West along Springfield    */
    R_CARIBOU,		/* the Caribou coffee shop           */
    R_KENNEY,		/* Kenney Gym                        */
    R_DCL,		/* Digital Computer Laboratory       */
    R_LIB_FRONT,	/* front of Grainger library         */

    /* Area 3: CSL and Environs */
    R_KENNEY_E,		/* East of Kenney Gym                */
    R_NEWMARK,		/* Newmark Laboratory                */
    R_MNTL_NW,		/* looking Northwest at MNTL         */
    R_MNTL_SW,		/* looking Southwest at MNTL         */
    R_MNTLLOBBY,	/* the lobby of MNTL                 */
    R_MNTL_LAB1,	/* a laboratory within MNTL (#1)     */
    R_MNTL_LAB2,	/* a laboratory within MNTL (#2)     */
    R_MNTL_LAB3,	/* a laboratory within MNTL (#3)     */
    R_CSL_VIEW, 	/* Coordinated Science Laboratory    */
    R_CSL_DOOR,		/* the CSL main entrance             */
    R_CSL_LOBBY,	/* in the CSL lobby                  */
    R_CSL_UPPER,	/* on an upper floor of CSL          */
    R_CSLLOUNGE,	/* in the new CSL lounge area        */
    R_BECK_LOT,		/* the Beckman Circle parking lot    */
    R_BECKMAN,		/* the Beckman Institute             */
    R_BECK_DOOR,	/* Beckman main entrance             */
    R_BECKLOBBY,	/* in the lobby of Beckman           */
    R_BECK_MRI,		/* an MRI machine ... somewhere      */

    /* Area 4: The Rest of the World, Featuring the Remote Sensing Lab */
    R_GARAGE,		/* the campus parking structure      */
    R_CAR_SITE,		/* the (fictitious) ECE391 car       */
    R_ALLERTON,		/* the Allerton mansion              */
    R_FU_DOGS,		/* the Fu dogs at Allerton           */
    R_STATUE,		/* a statue near the Fu dogs         */
    R_SUNSINGER,	/* the Allerton Sun Singer statue    */
    R_WILLARD,		/* Willard Airport fountain view     */
    R_WILL_SIDE,	/* side view of Willard and tower    */
    R_REM_PLANE,	/* a sensor-laden plane              */
    R_COCKPIT,		/* cockpit of remote sensing plane   */
    R_OVER_WILL,	/* flying above Willard Airport      */ 
    R_AIR_RIO,		/* view of Rio de Janeiro from air   */
    R_REM_ICE,		/* the ice fields near rem. sen. lab */
    R_REM_LAB,		/* part of a remote sensing lab      */

    N_ROOMS
};

/* object identifiers */
enum {
    O_NONE = -1,

    O_BOARD,		/* a motorized mountain board                 */
    O_JETPACK,		/* Buzz Lightyear: to Infinity ...            */
    O_TUX,		/* Tux: our mascot                            */
    O_MP2,		/* the MP2 specification (covers mode X)      */
    O_BOOK_C,		/* the C programming language                 */
    O_BOOK_WODE,	/* stories by P.G. Wodehouse                  */
    O_GPS_BAD,		/* a malfunctioning GPS device                */
    O_GPS_GOOD,		/* a working GPS device                       */
    O_GPS_SPEC,		/* GPS chip data sheet (specifications)       */
    O_BUNNYSUIT,	/* a pink bunny suit                          */
    O_BATT_EMPTY,	/* an uncharged car battery                   */
    O_BATT_FULL,	/* a fully charged car battery                */
    O_BATT_CAR,		/* battery as it appears in the car           */
    O_MTN_DEW,		/* a bottle of dew                            */
    O_FISH,		/* a fish to lure penguins                    */
    O_ICARD,		/* an I-card                                  */
    O_CAR_KEY,		/* the keys to a car                          */
    O_ROBOT_DEAD,	/* a buggy lockpicking robot                  */
    O_ROBOT_LIVE,	/* lockpicking robot with new control program */
    O_MIMO_CARD,	/* a MIMO card for planes                     */

    N_OBJECTS
};

/* flag identifiers for recording the player's accomplishments */
enum {
    FLAG_HAS_EATEN,	/* player has eaten something         */
    FLAG_WEARING_SUIT,	/* player is wearing a bunny suit     */
    FLAG_CAR_OPEN,	/* player has managed to open the car */
    FLAG_CAR_FIXED,	/* player has fixed the car           */
    FLAG_LURED_TUX, 	/* player has lured Tux to their side */
    NUM_FLAGS
};

/* identifiers for rooms with photo swapping */
enum {
    SWAP_CIRCLE,	/* Boneyard Creek Bridge photo swap */
    SWAP_CAR,		/* open/closed hood                 */
    N_SWAPS
};


/* types local to this file (declared in types.h) */

/*
 * The structure representing a room in the world.  The backpack/inventory 
 * is also a 'room' (#0, R_INVENTORY). 
 */
struct room_t {
    const char* name;		/* name of room                   */
    photo_t*    view;		/* photo currently shown for room */
    object_t*   contents; 	/* linked list of objects in room */
    room_t*     left;   	/* room to the "left"             */
    room_t*     enter;  	/* doors, etc.                    */
    room_t*     right;  	/* room to the "right"            */
};

/*
 * The structure representing an object in the world.  Objects are
 * unique, which prevents players from drinking too much Dew (they're
 * all the same bottle!).  Sorry.
 */
struct object_t {
    const char*  name;		/* name of object                 */
    object_t*    next;		/* linked list of room contents   */
    room_t*      loc;      	/* in what 'room'?                */
    uint16_t     x, y;    	/* location within room photo     */
    image_t*     img;     	/* image for use in room          */
};

/*
 * This local structure is used to specify room connectivity and data 
 * in a reasonably manageable way.  The array entries in the database
 * specify the room id, name, filename, and connections to other
 * rooms by symbolic name.  Since there are many rooms, trying to
 * keep the order between the enumeration of symbolic names and the
 * array entries can be tricky.  Instead, we associate array entries
 * with specific symbolic names using the id field.  Thus, you can
 * reorder the entries in the room_data array without affecting their
 * meaning.
 */
typedef struct room_data_t room_data_t;
struct room_data_t {
    int32_t           id;	/* id of room being described    */
    const char* const name;	/* name of room                  */
    const char* const filename;	/* file name for room photo      */
    int32_t           left;	/* id of room to 'left'          */
    int32_t           enter;	/* id of room reached by 'enter' */
    int32_t           right;	/* id of room to 'right'         */
};

/* the room connectivity data */
static const room_data_t room_data[N_ROOMS] = {
    /* Area 0: The Backpack */
    {R_INVENTORY, "Inventory", "images/backpack.photo", 
                       R_NONE,      R_NONE,      R_NONE},

    /* Area 1: Everitt and Green Street */
    {R_IN_391LAB, "391 Lab", "images/391lab.photo", 
                       R_NONE, R_BY_391LAB,      R_NONE},
    {R_BY_391LAB, "Outside of 391", "images/outside391.photo", 
                     R_BY_ZAS, R_IN_391LAB,   R_BY_IEEE},
    {  R_IN_IEEE, "IEEE Office", "images/ieee.photo", 
                       R_NONE,   R_BY_IEEE,      R_NONE},
    {  R_BY_IEEE, "Outside IEEE", "images/byieee.photo", 
                  R_BY_391LAB,   R_IN_IEEE, R_BY_395LAB},
    {R_IN_395LAB, "395 Lab", "images/395lab.photo", 
                       R_NONE, R_BY_395LAB,      R_NONE},
    {R_BY_395LAB, "Outside of 395", "images/outside395.photo", 
                    R_BY_IEEE,      R_NONE, R_EVT_STAIR},
    {R_EVT_STAIR, "Everitt Stairs", "images/evtstair.photo", 
                  R_BY_395LAB, R_EAST_EVRT, R_BY_CLEANR},
    {R_IN_CLEANR, "In Cleanroom", "images/cleanr.photo", 
                       R_NONE, R_BY_CLEANR,      R_NONE},
    {R_BY_CLEANR, "By the Cleanroom", "images/outclean.photo", 
                  R_EVT_STAIR,      R_NONE, R_EVRT_VEND},
    {R_EVRT_VEND, "Vending Machine", "images/vend.photo", 
                  R_BY_CLEANR, R_EVRT_BSMT,      R_NONE},
    {R_ALMAMATER, "Alma Mater", "images/almamater.photo", 
                  R_EAST_EVRT, R_EAST_EVRT, R_BY_COCOMR},
    {R_IN_COCOMR, "Cocomero", "images/incoco.photo", 
                       R_NONE, R_BY_COCOMR,      R_NONE},
    {R_BY_COCOMR, "Near Cocomero", "images/bycoco.photo", 
                  R_ALMAMATER, R_IN_COCOMR,    R_BY_ZAS},
    {R_BY_ZAS, "The Ruins", "images/ruins.photo", 
                  R_BY_COCOMR,      R_NONE,      R_NONE},
    {R_EAST_EVRT, "East of Everitt", "images/eeast.photo", 
                  R_ALMAMATER, R_EVT_STAIR, R_EVRT_BSMT},
    {R_EVRT_BSMT, "Basement Entry", "images/basement.photo", 
                  R_EAST_EVRT, R_EVRT_VEND, R_CIRCLE_SW},

    /* Area 2: Bardeen Quad and Environs */
    {R_WEST_BONE, "Boneyard Creek", "images/bonew.photo", 
                  R_CIRCLE_SW,      R_NONE,  R_CIRCLE_N},
    { R_CIRCLE_N, "Boneyard Bridge", "images/circlen1.photo", 
                  R_WEST_BONE, R_TALBOT_NW, R_EAST_BONE},
    {R_CIRCLE_SW, "Boneyard Bridge", "images/circlesw.photo", 
                  R_EAST_BONE, R_EVRT_BSMT,  R_CIRCLE_N},
    {R_EAST_BONE, "Boneyard Creek", "images/bonee.photo", 
                   R_CIRCLE_N,      R_NONE, R_CIRCLE_SW},
    {  R_BARDEEN, "Bardeen Quad", "images/bardeen.photo", 
                   R_LIB_BACK, R_EAST_BONE, R_TALBOT_SW},
    { R_LIB_BACK, "Grainger Library", "images/graingerback.photo", 
                        R_DCL,   R_RESERVE,   R_BARDEEN},
    {  R_RESERVE, "Grainger Reserves", "images/reserve.photo", 
                       R_NONE,  R_LIB_BACK, R_LIB_FRONT},
    {R_TALBOT_NW, "Talbot Lab", "images/talbotnw.photo", 
                  R_CIRCLE_SW,    R_TALBOT, R_TALBOT_SW},
    {R_TALBOT_SW, "Talbot Lab", "images/talbotsw.photo", 
                  R_TALBOT_NW,    R_TALBOT, R_SPRINGFLD},
    {   R_TALBOT, "Talbot Lab", "images/talbot.photo", 
                       R_NONE, R_TALBOT_NW,      R_NONE},
    {R_SPRINGFLD, "Springfield Avenue", "images/springfield.photo", 
                  R_TALBOT_SW,   R_CARIBOU,    R_KENNEY},
    {  R_CARIBOU, "Caribou", "images/caribou.photo", 
                       R_NONE, R_SPRINGFLD,      R_NONE},
    {   R_KENNEY, "Kenney Gym", "images/kenney.photo", 
                  R_SPRINGFLD,      R_NONE,       R_DCL},
    {      R_DCL, "DCL", "images/dcl.photo", 
                     R_KENNEY,  R_KENNEY_E, R_LIB_FRONT},
    {R_LIB_FRONT, "Grainger Library", "images/graingerfront.photo", 
                        R_DCL,   R_RESERVE, R_TALBOT_SW},

    /* Area 3: CSL and Environs */
    { R_KENNEY_E, "East of Kenney", "images/kenneye.photo", 
                        R_DCL,       R_DCL,   R_NEWMARK},
    {  R_NEWMARK, "Newmark Lab", "images/newmark.photo", 
                    R_MNTL_NW,      R_NONE,  R_KENNEY_E},
    {  R_MNTL_NW, "MNTL", "images/mntlnw.photo", 
                    R_NEWMARK, R_MNTLLOBBY,  R_CSL_VIEW},
    {  R_MNTL_SW, "MNTL", "images/mntlsw.photo", 
                    R_MNTL_NW, R_MNTLLOBBY,   R_BECKMAN},
    {R_MNTLLOBBY, "Lobby of MNTL", "images/mntllobby.photo", 
                  R_MNTL_LAB1,   R_MNTL_SW, R_MNTL_LAB2},
    {R_MNTL_LAB1, "Kevin's Lab in MNTL", "images/mntllab1.photo", 
                       R_NONE,      R_NONE, R_MNTLLOBBY},
    {R_MNTL_LAB2, "MNTL Laser Lab", "images/mntllab2.photo", 
                  R_MNTLLOBBY, R_MNTL_LAB3,      R_NONE},
    {R_MNTL_LAB3, "MNTL Laser Lab", "images/mntllab3.photo", 
                       R_NONE, R_MNTL_LAB2,      R_NONE},
    { R_CSL_VIEW, "CSL", "images/csl.photo", 
                   R_BECK_LOT,  R_CSL_DOOR,   R_MNTL_NW},
    { R_CSL_DOOR, "CSL Main Entrance", "images/csldoor.photo", 
                   R_BECK_LOT,      R_NONE,   R_MNTL_NW},
    {R_CSL_LOBBY, "CSL Lobby", "images/csllobby.photo", 
                  R_CSL_UPPER,  R_CSL_DOOR,      R_NONE},
    {R_CSL_UPPER, "Upper Floor of CSL", "images/cslupper.photo", 
                       R_NONE, R_CSLLOUNGE, R_CSL_LOBBY},
    {R_CSLLOUNGE, "CSL Lounge", "images/csllounge.photo", 
                       R_NONE, R_CSL_UPPER,      R_NONE},
    { R_BECK_LOT, "Beckman Circle Lot", "images/becklot.photo", 
                    R_BECKMAN,    R_GARAGE,  R_CSL_VIEW},
    {  R_BECKMAN, "Beckman Institute", "images/beckman.photo", 
                    R_MNTL_SW, R_BECK_DOOR,  R_BECK_LOT},
    {R_BECK_DOOR, "Beckman Institute", "images/beckdoor.photo", 
                    R_MNTL_SW,      R_NONE,  R_BECK_LOT},
    {R_BECKLOBBY, "Beckman Lobby", "images/becklobby.photo", 
                       R_NONE,  R_BECK_MRI, R_BECK_DOOR},
    { R_BECK_MRI, "An MRI Lab", "images/beckmri.photo", 
                       R_NONE, R_BECKLOBBY,      R_NONE},

    /* Area 4: The Rest of the World, Featuring the Remote Sensing Lab */
    {   R_GARAGE, "Campus Parking", "images/garage.photo", 
                   R_BECK_LOT,  R_CAR_SITE,      R_NONE},
    { R_CAR_SITE, "Use Someone's Car?", "images/carclosed.photo", 
                       R_NONE,    R_GARAGE,      R_NONE},
    { R_ALLERTON, "Allerton Mansion", "images/allerton.photo", 
                    R_FU_DOGS,      R_NONE, R_SUNSINGER},
    {  R_FU_DOGS, "Fu Dog Statues", "images/fudogs.photo", 
                       R_NONE,    R_STATUE,  R_ALLERTON},
    {   R_STATUE, "A Tall Statue", "images/statue.photo", 
                       R_NONE,   R_FU_DOGS,      R_NONE},
    {R_SUNSINGER, "The Sun Singer", "images/sunsinger.photo", 
                   R_ALLERTON,      R_NONE,      R_NONE},
    {  R_WILLARD, "Willard Airport", "images/willard.photo", 
                       R_NONE, R_WILL_SIDE,      R_NONE},
    {R_WILL_SIDE, "Willard Tower", "images/willardside.photo", 
                  R_REM_PLANE,      R_NONE,   R_WILLARD},
    {R_REM_PLANE, "Sensor-Laden Plane", "images/rsenseplane.photo", 
                    R_COCKPIT,      R_NONE, R_WILL_SIDE},
    {  R_COCKPIT, "Plane Cockpit", "images/cockpit.photo", 
                       R_NONE,      R_NONE, R_REM_PLANE},
    {R_OVER_WILL, "Flying over Willard", "images/overwillard.photo", 
                       R_NONE,   R_COCKPIT,   R_AIR_RIO},
    {  R_AIR_RIO, "Rio de Janeiro", "images/riofromair.photo", 
                  R_OVER_WILL,      R_NONE,   R_REM_ICE},
    {  R_REM_ICE, "Ice Fields", "images/rsenseice.photo", 
                    R_AIR_RIO,   R_REM_LAB,      R_NONE},
    {  R_REM_LAB, "Remote Sensing Lab", "images/rsenselab.photo", 
                       R_NONE,   R_REM_ICE,      R_NONE}
};

/*
 * This local structure is used to specify object information in a 
 * reasonably manageable way.  The array entries in the database specify 
 * the object id, name, filename, starting room (if any), and starting 
 * location * within that room (if not random).  We use this approach for 
 * the same * reason as with the rooms: to avoid the need to match the 
 * order between the enumeration of symbolic names and the array entries.
 */
typedef struct obj_data_t obj_data_t;
struct obj_data_t {
    int32_t id;			/* object identifier                   */
    const char* const name;	/* object keyword                      */
    const char* const filename; /* object image file name              */
    int32_t room;		/* starting room or R_NONE             */
    int32_t x;			/* starting x position (-1 for random) */
    int32_t y;			/* starting y position                 */
};

/* the object positioning data */
static const obj_data_t obj_data[N_OBJECTS] = {
    {     O_BOARD, "board", "images/board.obj", R_IN_IEEE, -1, -1},
    {   O_JETPACK, "jetpack", "images/jetpack.obj", R_TALBOT, -1, -1},
    {       O_TUX, "tux", "images/tux.obj", R_REM_LAB, 250, 100},
    {       O_MP2, "mp2", "images/mp2.obj", R_CSLLOUNGE, -1, -1},
    {    O_BOOK_C, "book", "images/book.obj", R_NONE, -1, -1},
    { O_BOOK_WODE, "book", "images/book2.obj", R_NONE, -1, -1},
    {   O_GPS_BAD, "gps", "images/gpsbad.obj", R_TALBOT, -1, -1},
    {  O_GPS_GOOD, "gps", "images/gpsgood.obj", R_NONE, -1, -1},
    {  O_GPS_SPEC, "spec", "images/gpsspec.obj", R_CSL_UPPER, -1, -1},
    { O_BUNNYSUIT, "bunnysuit", "images/bunnysuit.obj", R_ALMAMATER, 230, 250},
    {O_BATT_EMPTY, "battery", "images/battery.obj", R_NONE, -1, -1},
    { O_BATT_FULL, "battery", "images/battery.obj", R_NONE, -1, -1},
    {  O_BATT_CAR, "battery", "images/batteryincar.obj", R_NONE, -1, -1},
    {   O_MTN_DEW, "dew", "images/dew.obj", R_NONE, -1, -1},
    {      O_FISH, "fish", "images/fish.obj", R_EAST_BONE, 80, 260},
    {     O_ICARD, "Icard", "images/icard.obj", R_BARDEEN, -1, -1},
    {   O_CAR_KEY, "key", "images/key.obj", R_CARIBOU, -1, -1},
    {O_ROBOT_DEAD, "robot", "images/robot.obj", R_MNTL_LAB3, -1, -1},
    {O_ROBOT_LIVE, "robot", "images/robot.obj", R_NONE, -1, -1},
    { O_MIMO_CARD, "mimo", "images/mimo.obj", R_STATUE, -1, -1}
};

/*
 * Some rooms alternate between two photos.  For these rooms, we load the
 * image data for both photos once, but need an extra pointer in order to
 * keep track of the photo currently swapped out.  We use these swap data
 * to name and describe these extra photos.
 */
typedef struct swap_data_t swap_data_t;
struct swap_data_t {
    int32_t id;
    const char* const filename;
};

/* the swap photo descriptions */
static const swap_data_t swap_data[N_SWAPS] = {
    {SWAP_CIRCLE, "images/circlen2.photo"},	/* alternate for Boneyard */
    {SWAP_CAR, "images/caropen.photo"}		/* open/closed car photos */
};


/* functions local to this file--see function headers for details */
static void do_photo_swap (room_t* r, int32_t which);
static object_t* find_in_room (const room_t* r, const char* arg);
static void insert_object_at (object_t* o, room_t* r, int32_t x, int32_t y);
static void insert_object (object_t* o, room_t* r);
static void move_object_to_inventory (object_t* obj);
static object_t* obj_special_get (room_t* r, const char* arg);
static int32_t player_flag_is_set (int32_t fnum);
static void player_set_flag (int32_t fnum);
static void remove_object (object_t* o);


/* file-scope variables */
/* 
 * Flags are coded as bit vectors using an array of 32-bit words.  It's 
 * overkill for this game, but it's nice not to worry about the number of 
 * flags...
 */
static room_t   room[N_ROOMS];			     /* rooms                */
static object_t object[N_OBJECTS];		     /* objects              */
static uint32_t player_flags[(NUM_FLAGS + 31) / 32]; /* accomplishment flags */
static photo_t* swap_photo[N_SWAPS];                 /* swapping photos      */


/* 
 * do_photo_swap
 *   DESCRIPTION: Swap a room photo with another stored image.
 *   INPUTS: r -- the room into which the photo is swapped
 *	     which -- index into array of stored photos
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
do_photo_swap (room_t* r, int32_t which)
{
    photo_t* tmp;	/* temporary variable to help with swap */

    /* Swap the photos. */
    tmp               = r->view;
    r->view           = swap_photo[which];
    swap_photo[which] = tmp;
}


/* 
 * find_in_room
 *   DESCRIPTION: Find an object by name in a room.  The name must match
 *                exactly, although the match is not sensitive to case.
 *   INPUTS: r -- the room in which to look
 *           arg -- the name of the object (a string)
 *   OUTPUTS: none
 *   RETURN VALUE: a pointer to a matching object, or NULL if none is found
 *   SIDE EFFECTS: none
 */
static object_t* 
find_in_room (const room_t* r, const char* arg)
{
    object_t* obj;	/* index over room contents */

    /* Loop over objects in room. */
    for (obj = r->contents; NULL != obj; obj = obj->next) {

	/* If we find a matching object, return it. */
        if (0 == strcasecmp (arg, obj->name)) {
	    return obj;
	}
    }

    /* No object found: return NULL. */
    return NULL;
}


/* 
 * insert_object_at
 *   DESCRIPTION: Place an object at a specific (x,y) location in a room.
 *                The location refers to the placement of the object in
 *                the room's photo.
 *   INPUTS: o -- the object being placed
 *           r -- the room
 *           x -- the x position for the object
 *           y -- the y position for the object
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: takes the object out of its current location
 */
static void 
insert_object_at (object_t* o, room_t* r, int32_t x, int32_t y)
{
    /* Remove object from its current room, if any. */
    remove_object (o);

    /* Position the object within the new room. */
    o->x = x;
    o->y = y;

    /* Now add the object to the new room's contents. */
    o->loc = r;
    o->next = r->contents;
    r->contents = o;
}


/* 
 * insert_object
 *   DESCRIPTION: Insert object at a random position within a room.
 *   INPUTS: o -- the object being placed
 *           r -- the room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: takes the object out of its current location; sets 
 *                 the position of the object within the new room 
 *                 randomly
 */
static void 
insert_object (object_t* o, room_t* r)
{
    int32_t space;	/* room photo height in pixels            */
    int32_t range;	/* number of pixels in placement interval */
    int32_t xpos;	/* x position selected                    */
    int32_t ypos;       /* y position selected                    */
    int32_t img_ht;     /* object image height in pixels          */


    /* Choose a random x location. */
    range = photo_width (r->view) - image_width (o->img);
    xpos = (0 >= range ? 0 : (rand () % range));

    /* Place in the lowest quarter of the roo photo if the object fits... */
    space = photo_height (r->view);
    img_ht = image_height (o->img);
    range = space / 4 - img_ht;
    if (0 >= range) {
	/* Doesn't fit: try not to let the object fall off the bottom. */
        range = space - img_ht;
	ypos = (0 >= range ? 0 : (rand () % range));
    } else {
	ypos = (0 >= range ? 0 : (rand () % range) + (3 * space) / 4);
    }

    /* Now put the object into the room at the chosen location. */
    insert_object_at (o, r, xpos, ypos);
}


/* 
 * move_object_to_inventory
 *   DESCRIPTION: Move an object into the player's inventory.  Try to 
 *                place objects on a 3x3 grid for clarity, but place
 *                randomly if necessary.
 *   INPUTS: obj -- the object
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: takes the object out of its current location
 */
static void
move_object_to_inventory (object_t* obj)
{
    object_t* conf;	/* loop index over possible conflicts for a space */
    int32_t   x;	/* loop index for 3x3 grid x positions            */
    int32_t   y;	/* loop index for 3x3 grid y positions            */

    /* 
     * This approach is asymptotically slow (N^2), but there shouldn't be 
     * much in inventory, so it doesn't matter.
     */
    for (y = 10; 160 >= y; y += 50) {
        for (x = 10; 210 >= x; x += 100) {
	    for (conf = room[R_INVENTORY].contents; NULL != conf; 
	    	 conf = conf->next) {
	        if (x == conf->x && y == conf->y) {
		    break;
		}
	    }
	    if (NULL == conf) {
		insert_object_at (obj, &room[R_INVENTORY], x, y);
		return;
	    }
	}
    }

    /* Give up: place randomly in bottom quarter like a room. */
    insert_object (obj, &room[R_INVENTORY]);
}


/* 
 * obj_special_get
 *   DESCRIPTION: Handle special effects "get" commands, in which a player
 *                gets an object that is not represented as an object_t in
 *                the room's contents.
 *   INPUTS: r -- the room in which the "get" is performed
 *           arg -- the name of the object sought
 *   OUTPUTS: none
 *   RETURN VALUE: an object to be gotten by the player, or NULL for nothing
 *   SIDE EFFECTS: may move objects or show status messages
 */
static object_t*
obj_special_get (room_t* r, const char* arg)
{
    /* Get a book from the Grainger reference desk... */
    if (&room[R_RESERVE] == r && 0 == strcasecmp ("book", arg)) {
	/* can only get it once... */
	if (player_flag_is_set (FLAG_HAS_EATEN)) {
	    if (NULL == object[O_BOOK_C].loc) {
		show_status ("You check out the C book.");
		return &object[O_BOOK_C];
	    }
	} else {
	    if (NULL == object[O_BOOK_WODE].loc) {
		show_status ("Here's a nice Wodehouse collection.");
		return &object[O_BOOK_WODE];
	    }
	}
    }

    /* Pick up the car battery... */
    if (&room[R_CAR_SITE] == r && object[O_BATT_CAR].loc == r) {
        remove_object (&object[O_BATT_CAR]);
	return &object[O_BATT_EMPTY];
    }

    /* That's all, folks! */
    return NULL;
}


/* 
 * player_flag_is_set
 *   DESCRIPTION: Checks whether the player has accomplished a specified task.
 *   INPUTS: fnum -- the accomplishment identifier (a FLAG_*)
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if the player has accomplished the task, 0 if not
 *   SIDE EFFECTS: none
 */
static int32_t
player_flag_is_set (int32_t fnum)
{
    return (0 != (player_flags[fnum / 32] & (1UL << (fnum % 32))));
}


/* 
 * player_set_flag
 *   DESCRIPTION: Sets the flag indicating that the player has accomplished 
 *                a specified task.
 *   INPUTS: fnum -- the accomplishment identifier (a FLAG_*)
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
player_set_flag (int32_t fnum)
{
    player_flags[fnum / 32] |= (1UL << (fnum % 32));
}


/* 
 * remove_object
 *   DESCRIPTION: Take an object out of its current location, leaving it
 *                in limbo (NULL location).
 *   INPUTS: o -- the object
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void
remove_object (object_t* o)
{
    object_t** find;	/* loop index over pointers to objects in room */

    /* Is object already in limbo? */
    if (NULL != o->loc) {

	/* Remove from previous room (with safety check)... */
	for (find = &o->loc->contents; NULL != *find; find = &(*find)->next) {
	    if (o == *find) {
		/* We found the predecessor!  Unlink the object. */
	        *find = o->next;
		break;
	    }
	}

	/* Mark the object's location as NULL. */
	o->loc = NULL;
    }
}


/* 
 * obj_get_x
 *   DESCRIPTION: Get x position of object within containing room.
 *   INPUTS: obj -- pointer to the object
 *   OUTPUTS: none
 *   RETURN VALUE: the object obj's x position
 *   SIDE EFFECTS: none
 */
uint16_t
obj_get_x (const object_t* obj)
{
    return obj->x;
}


/* 
 * obj_get_y
 *   DESCRIPTION: Get y position of object within containing room.
 *   INPUTS: obj -- pointer to the object
 *   OUTPUTS: none
 *   RETURN VALUE: the object obj's y position
 *   SIDE EFFECTS: none
 */
uint16_t
obj_get_y (const object_t* obj)
{
    return obj->y;
}


/* 
 * obj_image
 *   DESCRIPTION: Get image pointer for object.
 *   INPUTS: obj -- pointer to the object
 *   OUTPUTS: none
 *   RETURN VALUE: the object obj's image pointer
 *   SIDE EFFECTS: none
 */
image_t*
obj_image (const object_t* obj)
{
    return obj->img;
}


/* 
 * obj_next
 *   DESCRIPTION: Get pointer to next object in object's room.  Use with
 *                room_contents_iterate to iterate over all objects in 
 *                a room.
 *   INPUTS: obj -- pointer to the object
 *   OUTPUTS: none
 *   RETURN VALUE: the object obj's next pointer (NULL if obj is last)
 *   SIDE EFFECTS: none
 */
object_t*
obj_next (const object_t* obj)
{
    return obj->next;
}


/* 
 * room_contents_iterate
 *   DESCRIPTION: Get pointer to the first object in a room.  Use with
 *                obj_next to iterate over all objects in a room.
 *   INPUTS: r -- pointer to the room
 *   OUTPUTS: none
 *   RETURN VALUE: a pointer to room r's first object (NULL when empty)
 *   SIDE EFFECTS: none
 */
object_t*
room_contents_iterate (const room_t* r)
{
    return r->contents;
}


/* 
 * room_name
 *   DESCRIPTION: Get name for a room.
 *   INPUTS: r -- pointer to the room
 *   OUTPUTS: none
 *   RETURN VALUE: the name of room r (a string)
 *   SIDE EFFECTS: none
 */
const char* 
room_name (const room_t* r)
{
    return r->name;
}


/* 
 * room_photo
 *   DESCRIPTION: Get room photo for a room.
 *   INPUTS: r -- pointer to the room
 *   OUTPUTS: none
 *   RETURN VALUE: a pointer to room r's photo
 *   SIDE EFFECTS: none
 */
photo_t*
room_photo (const room_t* r)
{
    return r->view;
}


/* 
 * room_photo_height
 *   DESCRIPTION: Get height of room photo in pixels for a room.
 *   INPUTS: r -- pointer to the room
 *   OUTPUTS: none
 *   RETURN VALUE: height of room r's photo in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
room_photo_height (const room_t* r)
{
    return photo_height (r->view);
}


/* 
 * room_photo_width
 *   DESCRIPTION: Get width of room photo in pixels for a room.
 *   INPUTS: r -- pointer to the room
 *   OUTPUTS: none
 *   RETURN VALUE: width of room r's photo in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
room_photo_width (const room_t* r)
{
    return photo_width (r->view);
}


/* 
 * build_world
 *   DESCRIPTION: Builds and connects the rooms, creates objects, and 
 *                reads in all image data (could be done lazily with 
 *                caching instead).
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 1 on success, or 0 on failure
 *   SIDE EFFECTS: prints error messages to stderr on failure
 */
int32_t
build_world ()
{
    int32_t idx;	/* index over data arrays   */
    int32_t which;	/* id for current data item */

    /* Clear all accomplishment flags. */
    (void)memset (player_flags, 0, sizeof (player_flags));

    /* Clear room data to enable sanity check for duplication. */
    (void)memset (room, 0, sizeof (room));

    /* Loop over room data. */
    for (idx = 0; N_ROOMS > idx; idx++) {
	
	/* Set the room id. */
	which = room_data[idx].id;

	/* Check for bad and duplicate ids. */
	if (0 > which || N_ROOMS <= which) {
	    fputs ("Bad index in room data.\n", stderr);
	    return 0;
	}
	if (NULL != room[which].name) {
	    fprintf (stderr, "Duplicate index %d in room data.\n", which);
	    return 0;
	}

	/* Set up the room. */
        room[which].name = room_data[idx].name;
	room[which].view = read_photo (room_data[idx].filename);
	if (NULL == room[which].view) {
	    fprintf (stderr, "Can't read room photo %s.\n", 
	    	     room_data[idx].filename);
	    return 0;
	}
	room[which].contents = NULL;
	room[which].left  = (R_NONE == room_data[idx].left ? NULL : 
			     &room[room_data[idx].left]);
	room[which].enter = (R_NONE == room_data[idx].enter ? NULL : 
			     &room[room_data[idx].enter]);
	room[which].right = (R_NONE == room_data[idx].right ? NULL : 
			     &room[room_data[idx].right]);
    }

    /* Clear object data to enable sanity check for duplication. */
    (void)memset (object, 0, sizeof (object));

    /* Loop over object data. */
    for (idx = 0; N_OBJECTS > idx; idx++) {

	/* Set the object id. */
	which = obj_data[idx].id;

	/* Check for bad and duplicate ids. */
	if (0 > which || N_OBJECTS <= which) {
	    fputs ("Bad index in object data.\n", stderr);
	    return 0;
	}
	if (NULL != object[which].name) {
	    fprintf (stderr, "Duplicate index %d in object data.\n", which);
	    return 0;
	}

	/* Set up the object. */
        object[which].name = obj_data[idx].name;
	object[which].img = read_obj_image (obj_data[idx].filename);
	if (NULL == object[which].img) {
	    fprintf (stderr, "Can't read object photo %s.\n", 
	    	     obj_data[idx].filename);
	    return 0;
	}
        object[which].next = NULL;
        object[which].loc = NULL;
        object[which].x = 0;
        object[which].y = 0;

	/* Insert it into a room if necessary. */
	if (R_NONE != obj_data[idx].room) {
	    if (-1 != obj_data[idx].x) {
	        insert_object_at (&object[which], &room[obj_data[idx].room],
				  obj_data[idx].x, obj_data[idx].y);
	    } else {
	        insert_object (&object[which], &room[obj_data[idx].room]);
	    }
	}
    }

    /* Clear swap photo data to enable sanity check for duplication. */
    (void)memset (swap_photo, 0, sizeof (swap_photo));

    /* Loop over swap photo data. */
    for (idx = 0; N_SWAPS > idx; idx++) {

	/* Set the swap photo id. */
	which = swap_data[idx].id;

	/* Check for bad and duplicate ids. */
	if (0 > which || N_SWAPS <= which) {
	    fputs ("Bad index in swap data.\n", stderr);
	    return 0;
	}
	if (NULL != swap_photo[which]) {
	    fprintf (stderr, "Duplicate index %d in swap data.\n", which);
	    return 0;
	}

	/* Read in the swap photo. */
	swap_photo[which] = read_photo (swap_data[idx].filename);
	if (NULL == swap_photo[which]) {
	    fprintf (stderr, "Can't read room photo %s.\n", 
	    	     swap_data[idx].filename);
	    return 0;
	}
    }

    /* Everything worked! */
    return 1;
}


/* 
 * start_in_room
 *   DESCRIPTION: Get a pointer to the room in which the player begins 
 *                the game.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to the starting room
 *   SIDE EFFECTS: none
 */
room_t*
start_in_room ()
{
    return &room[R_EAST_EVRT];
}


/* 
 * player_has_board
 *   DESCRIPTION: Check whether the player has the board in inventory.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if the board is in inventory, 0 if not
 *   SIDE EFFECTS: none
 */
int32_t
player_has_board ()
{
    return (&room[R_INVENTORY] == object[0].loc);
}


/* 
 * player_has_jetpack
 *   DESCRIPTION: Check whether the player has the jetpack in inventory.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if the jetpack is in inventory, 0 if not
 *   SIDE EFFECTS: none
 */
int32_t
player_has_jetpack ()
{
    return (&room[R_INVENTORY] == object[1].loc);
}


/* 
 * try_to_move_left
 *   DESCRIPTION: Try to move to the room to the 'left'.
 *   INPUTS: *rptr -- player's current room
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
try_to_move_left (room_t** rptr)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* If room exists, move into it. */
    if (NULL != r->left) {
        *rptr = r->left;

	/* When entering the Boneyard Circle, choose picture randomly. */
	if (&room[R_CIRCLE_N] == *rptr && 0 == (rand () % 2)) {
	    do_photo_swap (*rptr, SWAP_CIRCLE);
	}
	return TC_CHANGE_ROOM;
    }

    if (&room[0] == r) {
	/* Give a hint as to how to get out of inventory. */
        show_status ("Push 'home' or type 'inventory'.");
    } else {
	/* Let the player know that the move failed. */
	show_status ("You can't go that way.");
    }
    return TC_ALLOW_EDIT;
}


/* 
 * try_to_enter
 *   DESCRIPTION: Try to 'enter' a room from the current room.
 *   INPUTS: *rptr -- player's current room
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
try_to_enter (room_t** rptr)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* If room exists, move into it. */
    if (NULL != r->enter) {
        *rptr = r->enter;

	/* When entering the Boneyard Circle, choose picture randomly. */
	if (&room[R_CIRCLE_N] == *rptr && 0 == (rand () % 2)) {
	    do_photo_swap (*rptr, SWAP_CIRCLE);
	}
	return TC_CHANGE_ROOM;
    }

    /* 
     * Check special conditions for motion, allow motion if the 
     * conditions are met, and give hints when the conditions are 
     * not met. 
     */
    if (&room[R_BY_CLEANR] == r) {
	if (player_flag_is_set (FLAG_WEARING_SUIT)) {
	    *rptr = &room[R_IN_CLEANR];
	    return TC_CHANGE_ROOM;
	}
	show_status ("You're not wearing a bunnysuit!");
	return TC_ALLOW_EDIT;
    }
    if (&room[R_BY_395LAB] == r) {
	if (object[O_ICARD].loc == &room[R_INVENTORY]) {
	    show_status ("You swiped your Icard.");
	    *rptr = &room[R_IN_395LAB];
	    return TC_CHANGE_ROOM;
	}
	show_status ("You need a valid Icard.");
	return TC_ALLOW_EDIT;
    }
    if (&room[R_CSL_DOOR] == r) {
	if (object[O_ICARD].loc == &room[R_INVENTORY]) {
	    show_status ("You swiped your Icard.");
	    *rptr = &room[R_CSL_LOBBY];
	    return TC_CHANGE_ROOM;
	}
	show_status ("You need a valid Icard.");
	return TC_ALLOW_EDIT;
    }
    if (&room[R_BECK_DOOR] == r) {
	if (object[O_ROBOT_LIVE].loc == &room[R_INVENTORY]) {
	    show_status ("The robot hand picked the lock!");
	    *rptr = &room[R_BECKLOBBY];
	    return TC_CHANGE_ROOM;
	}
	if (object[O_ROBOT_DEAD].loc == &room[R_INVENTORY]) {
	    show_status ("Flash the robot's code again.");
	    return TC_ALLOW_EDIT;
	}
	show_status ("Complex lock!  Find a nanotech robot.");
	return TC_ALLOW_EDIT;
    }
    if (&room[R_MNTL_LAB1] == r) {
        /* Get advice from Kevin. */
	static const char* const advice[8] = {
	    "Kevin says, \"Andres' board is FAST!\"",
	    "Kevin asks, \"Got a jetpack?\"",
	    "Kevin: \"You can flash hardware in 395.\"",
	    "Kevin suggests, \"Always carry a fish.\"",
	    "Kevin asks, \"Have you done your MP2?\"",
	    "Kevin says, \"Cocomero sells yogurt.\"",
	    "Kevin asks, \"Maybe you need a Dew?\"",
	    "Kevin: \"A magnet can charge a battery.\""
	};
	show_status (advice[(rand () % 8)]);
	return TC_ALLOW_EDIT;
    }
    if (&room[R_COCKPIT] == r) {
        show_status ("A MIMO transmitter card is missing!");
	return TC_ALLOW_EDIT;
    }

    /* Let the player know that the move failed. */
    show_status ("You can't go that way.");
    return TC_ALLOW_EDIT;
}


/* 
 * try_to_move_right
 *   DESCRIPTION: Try to move to the room to the 'right'.
 *   INPUTS: *rptr -- player's current room
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
try_to_move_right (room_t** rptr)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* If room exists, move into it. */
    if (NULL != r->right) {
        *rptr = r->right;

	/* When entering the Boneyard Circle, choose picture randomly. */
	if (&room[R_CIRCLE_N] == *rptr && 0 == (rand () % 2)) {
	    do_photo_swap (*rptr, SWAP_CIRCLE);
	}
	return TC_CHANGE_ROOM;
    }

    if (&room[0] == r) {
	/* Give a hint as to how to get out of inventory. */
        show_status ("Push 'home' or type 'inventory'.");
    } else {
	/* Let the player know that the move failed. */
	show_status ("You can't go that way.");
    }
    return TC_ALLOW_EDIT;
}


/* 
 * typed_cmd_buy
 *   DESCRIPTION: Execute the typed command "buy," which allows the player
 *                to simulate purchase of objects (sometimes obtaining an
 *                a real object, sometimes an accomplishment flag).
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to buy
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_buy (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Buy a Dew! */
    if (0 == strcasecmp ("dew", arg)) {
        if (&room[R_EVRT_VEND] != r) {
	    show_status ("Great idea!  But ... where?");
	    return TC_DISCARD_TEXT;
	} 
	if (object[O_MTN_DEW].loc == &room[R_INVENTORY] ||
	    object[O_MTN_DEW].loc == r) {
	    show_status ("Slow down!  One at a time...");
	    return TC_DISCARD_TEXT;
	} 
	if (NULL != object[O_MTN_DEW].loc) {
	    show_status ("Last one get stolen?  Ok...here we go...");
	} else {
	    show_status ("You buy a Dew.");
	}
	move_object_to_inventory (&object[O_MTN_DEW]);
	return TC_REDRAW_ROOM;
    }

    /* Buy some yogurt. */
    if (0 == strcasecmp ("yogurt", arg)) {
        if (&room[R_IN_COCOMR] != r) {
	    show_status ("Cocomero doesn't deliver here.");
	} else if (player_flag_is_set (FLAG_HAS_EATEN)) {
	    show_status ("You're not hungry.");
	} else {
	    player_set_flag (FLAG_HAS_EATEN);
	    show_status ("So tasty and delicious!");
	}
	return TC_DISCARD_TEXT;
    }

    /* The player got too imaginative. */
    show_status ("Sorry, purchasing options are limited.");
    return TC_ALLOW_EDIT;
}


/* 
 * typed_cmd_charge
 *   DESCRIPTION: Execute the typed command "charge".
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to charge
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_charge (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Only the battery can be charged. */
    if (0 != strcasecmp ("battery", arg)) {
        show_status ("Electronic devices aren't (always) toys!");
	return TC_ALLOW_EDIT;
    }
    if (object[O_BATT_EMPTY].loc != &room[R_INVENTORY] &&
	object[O_BATT_EMPTY].loc != r &&
	object[O_BATT_FULL].loc != &room[R_INVENTORY] &&
	object[O_BATT_FULL].loc != r) {
	show_status ("What battery?");
	return TC_DISCARD_TEXT;
    }
    if (&room[R_BECK_MRI] != r) {
	show_status ("Find a bigger magnet.");
	return TC_DISCARD_TEXT;
    }
    if (object[O_BATT_FULL].loc == &room[R_INVENTORY] ||
	object[O_BATT_FULL].loc == r) {
	show_status ("Don't overdo it.");
	return TC_DISCARD_TEXT;
    }
    remove_object (&object[O_BATT_EMPTY]);
    move_object_to_inventory (&object[O_BATT_FULL]);
    show_status ("Wow!  That's a strong magnet!");
    return TC_REDRAW_ROOM;
}

/* 
 * typed_cmd_do
 *   DESCRIPTION: Execute the typed command "do," which allows the player
 *                to do certain things...like their 391 MP2!
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to do
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_do (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    if (&room[R_IN_391LAB] != r) {
        show_status ("You can't 'do' anything here.");
	return TC_ALLOW_EDIT;
    }
    if (0 != strcasecmp ("391", arg) &&
	0 != strcasecmp ("mp2", arg)) {
        show_status ("Doing the 391 MP2 is more important!");
	return TC_ALLOW_EDIT;
    }
    if (object[O_BOOK_C].loc != &room[R_INVENTORY]) {
        show_status ("You'd better get a book from Grainger.");
	return TC_DISCARD_TEXT;
    }
    if (object[O_MP2].loc != &room[R_INVENTORY]) {
        show_status ("Web's down.  Bring your own MP2.");
	return TC_DISCARD_TEXT;
    }
    if (object[O_TUX].loc != &room[R_IN_391LAB]) {
        show_status ("You'd have better luck if Tux were here.");
	return TC_DISCARD_TEXT;
    }

    /* The player wins the game! */
    *rptr = NULL;
    return TC_CHANGE_ROOM;
}


/* 
 * typed_cmd_drink
 *   DESCRIPTION: Execute the typed command "drink," which allows the player
 *                to drink objects.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to drink
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_drink (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* All you can drink is Dew... */
    if (0 != strcasecmp ("dew", arg)) {
        show_status ("That sounds less refreshing than Dew.");
	return TC_ALLOW_EDIT;
    }
    if (object[O_MTN_DEW].loc != &room[R_INVENTORY] &&
        object[O_MTN_DEW].loc != r) {
        show_status ("Uh-oh.  Hadewcinations.  Buy one soon!");
	return TC_DISCARD_TEXT;
    }
    remove_object (&object[O_MTN_DEW]);
    show_status ("Ahhhhhhhhhhhhhhhh...........nother?");
    /* NOT a bug.  Sorry, Dew doesn't count as a food. */
    return TC_REDRAW_ROOM;
}


/* 
 * typed_cmd_drop
 *   DESCRIPTION: Execute the typed command "drop," which allows the player
 *                to drop objects from their inventory into the room in
 *                which they're standing.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to drop
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_drop (room_t** rptr, const char* arg)
{
    room_t*   r;	/* current room                        */
    object_t* obj;      /* object being dropped                */
    room_t*   dest;	/* destination room for dropped object */

    /* Set current room. */
    r = *rptr;

    /* Search for object to drop--it must be in the player's inventory. */
    obj = find_in_room (&room[R_INVENTORY], arg);

    /* No luck--say so. */
    if (NULL == obj) {
	show_status ("You have no such thing.");
        return TC_ALLOW_EDIT;
    }
    
    /* 
     * Issue a warning to player if they seem to be trying to make use
     * of certain objects (as a hint).
     */
    if ((&object[O_BATT_FULL] == obj && &room[R_CAR_SITE] == r) ||
	(&object[O_MIMO_CARD] == obj && &room[R_REM_PLANE] == r)) {
        show_status ("You may want to install it instead.");
    }

    /* 
     * If player is looking at inventory, object goes into the room in 
     * which they're standing.
     */
    dest = (&room[R_INVENTORY] == r ? room[R_INVENTORY].enter : r);
    insert_object (obj, dest);
    return TC_REDRAW_ROOM;
}


/* 
 * typed_cmd_fix
 *   DESCRIPTION: Execute the typed command "fix," which allows the player
 *                to fix objects.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to fix
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_fix (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Only the GPS can be fixed. */
    if (0 != strcasecmp ("gps", arg)) {
        show_status ("In the game, you're not as capable.");
	return TC_ALLOW_EDIT;
    }
    if (object[O_GPS_GOOD].loc == &room[R_INVENTORY] ||
        object[O_GPS_GOOD].loc == r) {
        show_status ("It's working fine.");
	return TC_DISCARD_TEXT;
    }
    if (object[O_GPS_BAD].loc != &room[R_INVENTORY] &&
        object[O_GPS_BAD].loc != r) {
        show_status ("Do you have a GPS?");
	return TC_DISCARD_TEXT;
    }
    if (&room[R_IN_CLEANR] != r) {
        show_status ("You'd better go to the cleanroom.");
	return TC_DISCARD_TEXT;
    }
    if (object[O_GPS_SPEC].loc != &room[R_INVENTORY] &&
        object[O_GPS_SPEC].loc != r) {
        show_status ("Maybe you'd better get a spec?");
	return TC_DISCARD_TEXT;
    }
    remove_object (&object[O_GPS_BAD]);
    remove_object (&object[O_GPS_SPEC]);
    move_object_to_inventory (&object[O_GPS_GOOD]);
    show_status ("All done--wow, you're good!");
    return TC_CHANGE_ROOM;
}


/* 
 * typed_cmd_flash
 *   DESCRIPTION: Execute the typed command "flash," which allows the player
 *                to flash objects with code and so forth.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to flash
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_flash (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Only the robot can be flashed. */
    if (0 != strcasecmp ("robot", arg)) {
        show_status ("Don't waste your time.");
	return TC_ALLOW_EDIT;
    }
    if (object[O_ROBOT_DEAD].loc != &room[R_INVENTORY] &&
        object[O_ROBOT_DEAD].loc != r &&
	object[O_ROBOT_LIVE].loc != &room[R_INVENTORY] &&
        object[O_ROBOT_LIVE].loc != r) {
        show_status ("Maybe get the robot first?");
	return TC_DISCARD_TEXT;
    }
    if (&room[R_IN_395LAB] != r) {
        show_status ("With spit and a lemon?  Try the lab.");
	return TC_DISCARD_TEXT;
    }
    if (object[O_ROBOT_LIVE].loc == &room[R_INVENTORY] ||
        object[O_ROBOT_LIVE].loc == r) {
        show_status ("You flash the robot's ROM again.");
	return TC_DISCARD_TEXT;
    }
    remove_object (&object[O_ROBOT_DEAD]);
    move_object_to_inventory (&object[O_ROBOT_LIVE]);
    show_status ("You flash it with a lockpicking code.");
    return TC_REDRAW_ROOM;
}


/* 
 * typed_cmd_get
 *   DESCRIPTION: Execute the typed command "get," which allows the player
 *                to move objects in a room into their inventory.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to get
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_get (room_t** rptr, const char* arg)
{
    room_t*   r;	/* current room                  */
    room_t*   src;	/* source room for object search */
    object_t* obj;	/* object being sought           */

    /* Set current room. */
    r = *rptr;

    /* 
     * If player is looking at inventory, source room for object search 
     * is the room in which they're standing.
     */
    src = (&room[R_INVENTORY] == r ? room[R_INVENTORY].enter : r);

    /* Try a special effect search followed by a normal search. */
    if (NULL == (obj = obj_special_get (src, arg))) {
	obj = find_in_room (src, arg);
    } 
    if (NULL == obj) {
	show_status ("You see no such thing here.");
        return TC_ALLOW_EDIT;
    }

    /* The player can't grab Tux! */
    if (&object[O_TUX] == obj && !player_flag_is_set (FLAG_LURED_TUX)) {
        show_status ("Tux must choose you!  Try using a fish.");
	return TC_DISCARD_TEXT;
    }

    /* Move the object into the player's inventory. */
    move_object_to_inventory (obj);
    return TC_REDRAW_ROOM;
}


/* 
 * typed_cmd_go
 *   DESCRIPTION: Execute the typed command "go," which allows the player
 *                to go from one place to another using room features.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of location to which to go
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_go (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Try to go to Allerton Mansion. */
    if (0 == strcasecmp ("allerton", arg)) {
        if (&room[R_ALLERTON] == r) {
	    show_status ("Kazam!  You're at Allerton!");
	    return TC_DISCARD_TEXT;
	}
        if (&room[R_WILLARD] != r && &room[R_CAR_SITE] != r) {
	    show_status ("That's quite a hike.");
	    return TC_DISCARD_TEXT;
	}
	if (!player_flag_is_set (FLAG_CAR_FIXED)) {
	    if (player_flag_is_set (FLAG_CAR_OPEN)) {
		show_status ("The car isn't working.");
	    } else {
		show_status ("Do you want to use that car?");
	    }
	    return TC_DISCARD_TEXT;
	}
	if (object[O_GPS_GOOD].loc != &room[R_INVENTORY]) {
	    if (object[O_GPS_BAD].loc == &room[R_INVENTORY]) {
	        show_status ("That's a long road with a broken GPS.");
	    } else {
	        show_status ("You'll need a GPS to find that place.");
	    }
	    return TC_DISCARD_TEXT;
	}
	show_status ("You drive to Allerton Park.");
	*rptr = &room[R_ALLERTON];
	return TC_CHANGE_ROOM;
    }

    /* Try to go to Willard Airport. */
    if (0 == strcasecmp ("willard", arg) ||
	0 == strcasecmp ("airport", arg)) {
        if (&room[R_WILLARD] == r) {
	    show_status ("Kazap!  You're at Willard!");
	    return TC_DISCARD_TEXT;
	}
        if (&room[R_ALLERTON] != r && &room[R_CAR_SITE] != r) {
	    show_status ("That's quite a hike.");
	    return TC_DISCARD_TEXT;
	}
	if (!player_flag_is_set (FLAG_CAR_FIXED)) {
	    if (player_flag_is_set (FLAG_CAR_OPEN)) {
		show_status ("The car isn't working.");
	    } else {
		show_status ("Do you want to use that car?");
	    }
	    return TC_DISCARD_TEXT;
	}
	show_status ("You drive to Willard Airport.");
	*rptr = &room[R_WILLARD];
	return TC_CHANGE_ROOM;
    }

    /* Try to go to campus. */
    if (0 == strcasecmp ("campus", arg)) {
        if (&room[R_CAR_SITE] == r) {
	    show_status ("Kazar!  You're on campus!");
	    return TC_DISCARD_TEXT;
	}
        if (&room[R_ALLERTON] != r && &room[R_WILLARD] != r) {
	    show_status ("That's quite a hike.");
	    return TC_DISCARD_TEXT;
	}
	show_status ("You drive back to campus.");
	*rptr = &room[R_CAR_SITE];
	return TC_CHANGE_ROOM;
    }

    /* Location unrecognized.  Say so. */
    show_status ("The game map lacks certain places.");
    return TC_ALLOW_EDIT;
}


/* 
 * typed_cmd_install
 *   DESCRIPTION: Execute the typed command "install," which allows the player
 *                to install objects.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to install
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_install (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Try to install a battery. */
    if (0 == strcasecmp ("battery", arg)) {
	if (object[O_BATT_EMPTY].loc != &room[R_INVENTORY] &&
	    object[O_BATT_EMPTY].loc != r &&
	    object[O_BATT_FULL].loc != &room[R_INVENTORY] &&
	    object[O_BATT_FULL].loc != r) {
	    show_status ("What battery?");
	    return TC_DISCARD_TEXT;
	}
	if (&room[R_CAR_SITE] != r) {
	    show_status ("Do you see the car?");
	    return TC_DISCARD_TEXT;
	}
	if (object[O_BATT_EMPTY].loc == &room[R_INVENTORY] ||
	    object[O_BATT_EMPTY].loc == r) {
	    show_status ("You want to install a dead battery?");
	    return TC_DISCARD_TEXT;
        }
	remove_object (&object[O_BATT_FULL]);
	player_set_flag (FLAG_CAR_FIXED);
	do_photo_swap (r, SWAP_CAR);
	show_status ("Nice work!  Now you can use it!");
	return TC_CHANGE_ROOM;
    }

    /* Try to install a MIMO transmitter card. */
    if (0 == strcasecmp ("mimo", arg) || 0 == strcasecmp ("card", arg) ||
	0 == strcasecmp ("transmitter", arg)) {
	if (object[O_MIMO_CARD].loc != &room[R_INVENTORY] &&
	    object[O_MIMO_CARD].loc != r) {
	    show_status ("Do you have one of those?");
	    return TC_DISCARD_TEXT;
	}
	if (&room[R_COCKPIT] != r) {
	    show_status ("Nothing here needs that.");
	    return TC_DISCARD_TEXT;
	}
	remove_object (&object[O_MIMO_CARD]);
	room[R_COCKPIT].enter = &room[R_OVER_WILL];
	show_status ("Ready for takeoff, captain!");
	return TC_REDRAW_ROOM;
    }

    /* There's nothing else that can be installed. */
    show_status ("What are you playing at?");
    return TC_ALLOW_EDIT;
}


/* 
 * typed_cmd_inventory
 *   DESCRIPTION: Execute the typed command "inventory," which moves the
 *                player to their inventory (room #0, or R_INVENTORY) in
 *                order to inspect their carried objects.  The current 
 *                room is saved in the inventory room's 'enter' direction.
 *
 *                Typing 'inventory' from the inventory view returns the
 *                player to the room in which they're standing.
 *
 *   INPUTS: *rptr -- player's current room
 *           arg -- takes no argument, so this parameter should be an 
 *                  empty string (we don't check)
 *   OUTPUTS: *rptr -- new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: changes player's room
 */
tc_action_t
typed_cmd_inventory (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    if (&room[R_INVENTORY] == r) {
	/* Return from inventory to previous room. */
	*rptr = r->enter;
    } else {
	/* Record current room and enter inventory view. */
	room[R_INVENTORY].enter = r;
	*rptr = &room[R_INVENTORY];
    }
    return TC_CHANGE_ROOM;
}


/* 
 * typed_cmd_sigh
 *   DESCRIPTION: Execute the typed command "sigh," which allows the player
 *                to show their respects for many a vanished site.  Ouch,
 *                sorry WS.
 *   INPUTS: *rptr -- player's current room
 *           arg -- takes no argument, so this parameter should be an 
 *                  empty string (we don't check)
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_sigh (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;
    if (&room[R_BY_ZAS] != r) {
        show_status ("MP2 got you down?  Take a break!");
    } else {
	show_status ("So sad... you lose your appetite.");
	player_set_flag (FLAG_HAS_EATEN);
    }
    return TC_DISCARD_TEXT;
}


/* 
 * typed_cmd_use
 *   DESCRIPTION: Execute the typed command "use," which allows the player
 *                to use objects in a room or in their inventory.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to use
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_use (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Try to use a car. */
    if (0 == strcasecmp ("car", arg)) {
    	if (&room[R_ALLERTON] == r) {
	    show_status ("Go to campus or Willard Airport?");
	    return TC_DISCARD_TEXT;
	}
    	if (&room[R_WILLARD] == r) {
	    show_status ("Go to Allerton or campus?");
	    return TC_DISCARD_TEXT;
	}
	if (&room[R_CAR_SITE] != r) {
	    show_status ("You have a car?");
	    return TC_DISCARD_TEXT;
	}
	if (player_flag_is_set (FLAG_CAR_FIXED)) {
	    show_status ("Go to Allerton or Willard Airport?");
	    return TC_DISCARD_TEXT;
	}
	if (player_flag_is_set (FLAG_CAR_OPEN)) {
	    show_status ("You'll have to charge the battery.");
	    return TC_DISCARD_TEXT;
	}
	if (object[O_CAR_KEY].loc != &room[R_INVENTORY]) {
	    show_status ("Perhaps you can find a key?");
	    return TC_DISCARD_TEXT;
	}
	do_photo_swap (r, SWAP_CAR);
	remove_object (&object[O_CAR_KEY]);
	insert_object_at (&object[O_BATT_CAR], r, 265, 122);
	player_set_flag (FLAG_CAR_OPEN);
	show_status ("The key works, but the battery's dead.");
	return TC_CHANGE_ROOM;
    }

    /* Try to use a fish. */
    if (0 == strcasecmp ("fish", arg)) {
	if (object[O_FISH].loc != &room[R_INVENTORY] &&
	    object[O_FISH].loc != r) {
	    show_status ("Using the invisible fish...no effect!");
	    return TC_DISCARD_TEXT;
	}
	if (&room[R_REM_LAB] != r) {
	    show_status ("I don't think that's sanitary.");
	    return TC_DISCARD_TEXT;
	}
	remove_object (&object[O_FISH]);
	move_object_to_inventory (&object[O_TUX]);
	player_set_flag (FLAG_LURED_TUX);
        show_status ("Tux likes you!");
	return TC_REDRAW_ROOM;
    }

    /* Don't ask.  Oh, was that YOU who tried to use that?  Uh. */
    show_status ("You want to use what!?");
    return TC_ALLOW_EDIT;
}


/* 
 * typed_cmd_wear
 *   DESCRIPTION: Execute the typed command "wear," which allows the player
 *                to wear objects.
 *   INPUTS: *rptr -- player's current room
 *           arg -- name of object to wear
 *   OUTPUTS: *rptr -- possibly new room for player
 *   RETURN VALUE: indicates types of action taken (see header file)
 *   SIDE EFFECTS: may move objects, show status messages, change player's room
 */
tc_action_t
typed_cmd_wear (room_t** rptr, const char* arg)
{
    room_t* r;	/* current room */

    /* Set current room. */
    r = *rptr;

    /* Only the bunnysuit can be worn. */
    if (0 != strcasecmp ("bunnysuit", arg)) {
        show_status ("Big Brother forbids fashion statements.");
	return TC_ALLOW_EDIT;
    }
    if (object[O_BUNNYSUIT].loc != &room[R_INVENTORY] &&
        object[O_BUNNYSUIT].loc != r) {
        show_status ("Do you have a bunnysuit?");
	return TC_DISCARD_TEXT;
    }
    remove_object (&object[O_BUNNYSUIT]);
    player_set_flag (FLAG_WEARING_SUIT);
    show_status ("You look good in pink!");
    return TC_REDRAW_ROOM;
}

