/*									tab:8
 *
 * modex.c - VGA mode X graphics routines
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
 * Version:	    5
 * Creation Date:   Fri Sep 10 09:59:17 2004
 * Filename:	    modex.c
 * History:
 *	SL	1	Fri Sep 10 09:59:17 2004
 *		First written.
 *	SL	2	Sat Sep 12 16:41:45 2009
 *		Integrated original release back into main code base.
 *	SL	3	Sat Sep 12 17:58:20 2009
 *              Added display re-enable to VGA blank routine and comments
 *              on other VirtualPC->QEMU migration changes.
 *	SL	4	Sat Sep 10 20:43:47 2011
 *		Modified for MP2 F11 adventure game.
 *	SL	5	Sat Sep 14 16:13:20 2011
 *		Split fill_palette by mode and cleaned up code for release.
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <unistd.h>

#include "modex.h"
#include "text.h"


/*
 * Calculate the image build buffer parameters.  SCROLL_SIZE is the space
 * needed for one plane of an image.  SCREEN_SIZE is the space needed for
 * all four planes.  The extra +1 supports logical view x coordinates that
 * are not multiples of four.  In these cases, some plane addresses are
 * shifted by 1 byte forward.  The planes are stored in the build buffer
 * in reverse order to allow those planes that shift forward to do so
 * without running into planes that aren't shifted.  For example, when
 * the leftmost x pixel in the logical view is 3 mod 4, planes 2, 1, and 0
 * are shifted forward, while plane 3 is not, so there is one unused byte
 * between the image of plane 3 and that of plane 2.  BUILD_BUF_SIZE is
 * the size of the space allocated for building images.  We add 20000 bytes
 * to reduce the number of memory copies required during scrolling.
 * Strictly speaking (try it), no extra space is necessary, but the minimum
 * means an extra 64kB memory copy with every scroll pixel.  Finally,
 * BUILD_BASE_INIT places initial (or transferred) logical view in the
 * middle of the available buffer area.
 */
#define SCROLL_SIZE     (SCROLL_X_WIDTH * SCROLL_Y_DIM)
#define SCREEN_SIZE     (SCROLL_SIZE * 4 + 1)
#define BUILD_BUF_SIZE  (SCREEN_SIZE + 20000)
#define BUILD_BASE_INIT ((BUILD_BUF_SIZE - SCREEN_SIZE) / 2)

/* Mode X and general VGA parameters */
#define VID_MEM_SIZE       131072
#define MODE_X_MEM_SIZE     65536
#define NUM_SEQUENCER_REGS      5
#define NUM_CRTC_REGS          25
#define NUM_GRAPHICS_REGS       9
#define NUM_ATTR_REGS          22

const int STATUS_BAR_OFFSET = 200 * 18;

/* VGA register settings for mode X */
static unsigned short mode_X_seq[NUM_SEQUENCER_REGS] = {
    0x0100, 0x2101, 0x0F02, 0x0003, 0x0604
};
static unsigned short mode_X_CRTC[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5404, 0x8005, 0xBF06, 0x1F07,
    0x0008, 0x0109, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x0014, 0x9615, 0xB916, 0xE317,
    0x6B18
};
static unsigned char mode_X_attr[NUM_ATTR_REGS * 2] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
    0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
    0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
    0x10, 0x41, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x00,
    0x14, 0x00, 0x15, 0x00
};
static unsigned short mode_X_graphics[NUM_GRAPHICS_REGS] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x4005, 0x0506, 0x0F07,
    0xFF08
};

/* VGA register settings for text mode 3 (color text) */
static unsigned short text_seq[NUM_SEQUENCER_REGS] = {
    0x0100, 0x2001, 0x0302, 0x0003, 0x0204
};
static unsigned short text_CRTC[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5504, 0x8105, 0xBF06, 0x1F07,
    0x0008, 0x4F09, 0x0D0A, 0x0E0B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x1F14, 0x9615, 0xB916, 0xA317,
    0xFF18
};
static unsigned char text_attr[NUM_ATTR_REGS * 2] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
    0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
    0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
    0x10, 0x0C, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x08,
    0x14, 0x00, 0x15, 0x00
};
static unsigned short text_graphics[NUM_GRAPHICS_REGS] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x1005, 0x0E06, 0x0007,
    0xFF08
};


/* local functions--see function headers for details */
static int open_memory_and_ports ();
static void VGA_blank (int blank_bit);
static void set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS],
        unsigned char val);
static void set_CRTC_registers (unsigned short table[NUM_CRTC_REGS]);
static void set_attr_registers (unsigned char table[NUM_ATTR_REGS * 2]);
static void set_graphics_registers (unsigned short table[NUM_GRAPHICS_REGS]);
static void fill_palette_mode_x ();
static void fill_palette_text ();
static void write_font_data ();
static void set_text_mode_3 (int clear_scr);
static void copy_image (unsigned char* img, unsigned short scr_addr, unsigned short num);


/*
 * Images are built in this buffer, then copied to the video memory.
 * Copying to video memory with REP MOVSB is vastly faster than anything
 * else with emulation, probably because it is a single instruction
 * and translates to a native loop.  It's also a pretty good technique
 * in normal machines (albeit not as elegant as some others for reducing
 * the number of video memory writes; unfortunately, these techniques
 * are slower in emulation...).
 *
 * The size allows the four plane images to move within an area of
 * about twice the size necessary (to reduce the need to deal with
 * the boundary conditions by moving the data within the buffer).
 *
 * Plane 3 is first, followed by 2, 1, and 0.  The reverse ordering
 * is used because the logical address of 0 increases first; if plane
 * 0 were first, we would need a buffer byte to keep it from colliding
 * with plane 1 when plane 0 was offset by 1 from plane 1, i.e., when
 * displaying a one-pixel left shift.
 *
 * The memory fence (included when NDEBUG is not defined) allocates
 * the build buffer with extra space on each side.  The extra space
 * is filled with magic numbers (something unlikely to be written in
 * error), and the fence areas are checked for those magic values at
 * the end of the program to detect array access bugs (writes past
 * the ends of the build buffer).
 */
#if !defined(NDEBUG)
#define MEM_FENCE_WIDTH 256
#else
#define MEM_FENCE_WIDTH 0
#endif
#define MEM_FENCE_MAGIC 0xF3
static unsigned char build[BUILD_BUF_SIZE + 2 * MEM_FENCE_WIDTH];
static int img3_off;		    /* offset of upper left pixel   */
static unsigned char* img3;	    /* pointer to upper left pixel  */
static int show_x, show_y;          /* logical view coordinates     */

/* displayed video memory variables */
static unsigned char* mem_image;    /* pointer to start of video memory */
static unsigned short target_img;   /* offset of displayed screen image */


/*
 * functions provided by the caller to set_mode_X() and used to obtain
 * graphic images of lines (pixels) to be mapped into the build buffer
 * planes for display in mode X
 */
static void (*horiz_line_fn) (int, int, unsigned char[SCROLL_X_DIM]);
static void (*vert_line_fn) (int, int, unsigned char[SCROLL_Y_DIM]);


/*
 * macro used to target a specific video plane or planes when writing
 * to video memory in mode X; bits 8-11 in the mask_hi_bits enable writes
 * to planes 0-3, respectively
 */
#define SET_WRITE_MASK(mask_hi_bits)                                    \
    do {                                                                    \
        asm volatile ("                                                     \
                movw $0x03C4,%%dx    	/* set write mask                    */;\
                movb $0x02,%b0                                                 ;\
                outw %w0,(%%dx)                                                 \
                " : : "a" ((mask_hi_bits)) : "edx", "memory");                      \
    } while (0)

                /* macro used to write a byte to a port */
#define OUTB(port,val)                                                  \
                do {                                                                    \
                asm volatile ("                                                     \
                        outb %b1,(%w0)                                                  \
                        " : /* no outputs */                                                \
                        : "d" ((port)), "a" ((val))                                       \
                        : "memory", "cc");                                                \
                } while (0)

                /* macro used to write two bytes to two consecutive ports */
#define OUTW(port,val)                                                  \
                do {                                                                    \
                asm volatile ("                                                     \
                        outw %w1,(%w0)                                                  \
                        " : /* no outputs */                                                \
                        : "d" ((port)), "a" ((val))                                       \
                        : "memory", "cc");                                                \
                } while (0)

/*
 * macro used to write an array of two-byte values to two consecutive ports
 */
#define REP_OUTSW(port,source,count)                                    \
    do {                                                                    \
        asm volatile ("                                                     \
                1: movw 0(%1),%%ax                                                ;\
                outw %%ax,(%w2)                                                ;\
                addl $2,%1                                                     ;\
                decl %0                                                        ;\
                jne 1b                                                          \
                " : /* no outputs */                                                \
                : "c" ((count)), "S" ((source)), "d" ((port))                     \
                : "eax", "memory", "cc");                                         \
    } while (0)

/*
 * macro used to write an array of one-byte values to two consecutive ports
 */
#define REP_OUTSB(port,source,count)                                    \
    do {                                                                    \
        asm volatile ("                                                     \
                1: movb 0(%1),%%al                                                ;\
                outb %%al,(%w2)                                                ;\
                incl %1                                                        ;\
                decl %0                                                        ;\
                jne 1b                                                          \
                " : /* no outputs */                                                \
                : "c" ((count)), "S" ((source)), "d" ((port))                     \
                : "eax", "memory", "cc");                                         \
    } while (0)


/*
 * set_mode_X
 *   DESCRIPTION: Puts the VGA into mode X.
 *   INPUTS: horiz_fill_fn -- this function is used as a callback (by
 *   			      draw_horiz_line) to obtain a graphical
 *   			      image of a particular logical line for
 *   			      drawing to the build buffer
 *           vert_fill_fn -- this function is used as a callback (by
 *   			     draw_vert_line) to obtain a graphical
 *   			     image of a particular logical line for
 *   			     drawing to the build buffer
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: initializes the logical view window; maps video memory
 *                 and obtains permission for VGA ports; clears video memory
 */
    int
set_mode_X (void (*horiz_fill_fn) (int, int, unsigned char[SCROLL_X_DIM]),
        void (*vert_fill_fn) (int, int, unsigned char[SCROLL_Y_DIM]))
{
    int i; /* loop index for filling memory fence with magic numbers */

    /*
     * Record callback functions for obtaining horizontal and vertical
     * line images.
     */
    if (horiz_fill_fn == NULL || vert_fill_fn == NULL)
        return -1;
    horiz_line_fn = horiz_fill_fn;
    vert_line_fn = vert_fill_fn;

    /* Initialize the logical view window to position (0,0). */
    show_x = show_y = 0;
    img3_off = BUILD_BASE_INIT;
    img3 = build + img3_off + MEM_FENCE_WIDTH;

    /* Set up the memory fence on the build buffer. */
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
        build[i] = MEM_FENCE_MAGIC;
        build[BUILD_BUF_SIZE + MEM_FENCE_WIDTH + i] = MEM_FENCE_MAGIC;
    }

    /* One display page goes at the start of video memory. */
    target_img = STATUS_BAR_OFFSET;

    /* Map video memory and obtain permission for VGA port access. */
    if (open_memory_and_ports () == -1)
        return -1;

    /*
     * The code below was produced by recording a call to set mode 0013h
     * with display memory clearing and a windowed frame buffer, then
     * modifying the code to set mode X instead.  The code was then
     * generalized into functions...
     *
     * modifications from mode 13h to mode X include...
     *   Sequencer Memory Mode Register: 0x0E to 0x06 (0x3C4/0x04)
     *   Underline Location Register   : 0x40 to 0x00 (0x3D4/0x14)
     *   CRTC Mode Control Register    : 0xA3 to 0xE3 (0x3D4/0x17)
     */

    VGA_blank (1);                               /* blank the screen      */
    set_seq_regs_and_reset (mode_X_seq, 0x63);   /* sequencer registers   */
    set_CRTC_registers (mode_X_CRTC);            /* CRT control registers */
    set_attr_registers (mode_X_attr);            /* attribute registers   */
    set_graphics_registers (mode_X_graphics);    /* graphics registers    */
    fill_palette_mode_x ();			 /* palette colors        */
    clear_screens ();				 /* zero video memory     */
    VGA_blank (0);			         /* unblank the screen    */

    /* Return success. */
    return 0;
}


/*
 * clear_mode_X
 *   DESCRIPTION: Puts the VGA into text mode 3 (color text).
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: restores font data to video memory; clears screens;
 *                 unmaps video memory; checks memory fence integrity
 */
    void
clear_mode_X ()
{
    int i;   /* loop index for checking memory fence */

    /* Put VGA into text mode, restore font data, and clear screens. */
    set_text_mode_3 (1);

    /* Unmap video memory. */
    (void)munmap (mem_image, VID_MEM_SIZE);

    /* Check validity of build buffer memory fence.  Report breakage. */
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
        if (build[i] != MEM_FENCE_MAGIC) {
            puts ("lower build fence was broken");
            break;
        }
    }
    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
        if (build[BUILD_BUF_SIZE + MEM_FENCE_WIDTH + i] != MEM_FENCE_MAGIC) {
            puts ("upper build fence was broken");
            break;
        }
    }
}


/*
 * set_view_window
 *   DESCRIPTION: Set the logical view window, moving its location within
 *                the build buffer if necessary to keep all on-screen data
 *                in the build buffer.  If the location within the build
 *                buffer moves, this function copies all data from the old
 *                window that are within the new screen to the appropriate
 *                new location, so only data not previously on the screen
 *                must be drawn before calling show_screen.
 *   INPUTS: (scr_x,scr_y) -- new upper left pixel of logical view window
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: may shift position of logical view window within build
 *                 buffer
 */
    void
set_view_window (int scr_x, int scr_y)
{
    int old_x, old_y;     /* old position of logical view window           */
    int start_x, start_y; /* starting position for copying from old to new */
    int end_x, end_y;     /* ending position for copying from old to new   */
    int start_off;        /* offset of copy start relative to old build    */
    /*    buffer start position                      */
    int length;           /* amount of data to be copied                   */
    int i;	          /* copy loop index                               */
    unsigned char* start_addr;  /* starting memory address of copy     */
    unsigned char* target_addr; /* destination memory address for copy */

    /* Record the old position. */
    old_x = show_x;
    old_y = show_y;

    /* Keep track of the new view window. */
    show_x = scr_x;
    show_y = scr_y;

    /*
     * If the new view window fits within the boundaries of the build
     * buffer, we need move nothing around.
     */
    if (img3_off + (scr_x >> 2) + scr_y * SCROLL_X_WIDTH >= 0 &&
            img3_off + 3 * SCROLL_SIZE +
            ((scr_x + SCROLL_X_DIM - 1) >> 2) +
            (scr_y + SCROLL_Y_DIM - 1) * SCROLL_X_WIDTH < BUILD_BUF_SIZE)
        return;

    /*
     * If the new screen does not overlap at all with the old screen, none
     * of the old data need to be saved, and we can simply reposition the
     * valid window of the build buffer in the middle of that buffer.
     */
    if (scr_x <= old_x - SCROLL_X_DIM || scr_x >= old_x + SCROLL_X_DIM ||
            scr_y <= old_y - SCROLL_Y_DIM || scr_y >= old_y + SCROLL_Y_DIM) {
        img3_off = BUILD_BASE_INIT - (scr_x >> 2) - scr_y * SCROLL_X_WIDTH;
        img3 = build + img3_off + MEM_FENCE_WIDTH;
        return;
    }

    /*
     * Any still-visible portion of the old screen should be retained.
     * Rather than clipping exactly, we copy all contiguous data between
     * a clipped starting point to a clipped ending point (which may
     * include non-visible data).
     *
     * The starting point is the maximum (x,y) coordinates between the
     * new and old screens.  The ending point is the minimum (x,y)
     * coordinates between the old and new screens (offset by the screen
     * size).
     */
    if (scr_x > old_x) {
        start_x = scr_x;
        end_x = old_x;
    } else {
        start_x = old_x;
        end_x = scr_x;
    }
    end_x += SCROLL_X_DIM - 1;
    if (scr_y > old_y) {
        start_y = scr_y;
        end_y = old_y;
    } else {
        start_y = old_y;
        end_y = scr_y;
    }
    end_y += SCROLL_Y_DIM - 1;

    /*
     * We now calculate the starting and ending addresses for the copy
     * as well as the new offsets for use with the build buffer.  The
     * length to be copied is basically the ending offset minus the starting
     * offset plus one (plus the three screens in between planes 3 and 0).
     */
    start_off = (start_x >> 2) + start_y * SCROLL_X_WIDTH;
    start_addr = img3 + start_off;
    length = (end_x >> 2) + end_y * SCROLL_X_WIDTH + 1 - start_off +
        3 * SCROLL_SIZE;
    img3_off = BUILD_BASE_INIT - (show_x >> 2) - show_y * SCROLL_X_WIDTH;
    img3 = build + img3_off + MEM_FENCE_WIDTH;
    target_addr = img3 + start_off;

    /*
     * Copy the relevant portion of the screen from the old location to the
     * new one.  The areas may overlap, so copy direction is important.
     * (You should be able to explain why!)
     */
    if (start_addr < target_addr)
        for (i = length; i-- > 0; )
            target_addr[i] = start_addr[i];
    else
        for (i = 0; i < length; i++)
            target_addr[i] = start_addr[i];
}

/* show_status_bar
 *   DESCRIPTION: Show the status bar window on the split display.
 *   INPUTS: buf - image buffer
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: writes into video memory;*/
    void
show_status_bar (unsigned char *buf)
{
    int i;
    // split into 4 planes
    for (i = 0; i < 4; i++)
    {
        SET_WRITE_MASK(1 << (i + 8));
        copy_image(buf + STATUS_BAR_OFFSET * i, 0, STATUS_BAR_OFFSET);
    }
}

/*
 * show_screen
 *   DESCRIPTION: Show the logical view window on the video display.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies from the build buffer to video memory;
 *                 shifts the VGA display source to point to the new image
 */
    void
show_screen ()
{
    unsigned char* addr;  /* source address for copy             */
    int p_off;            /* plane offset of first display plane */
    int i;		  /* loop index over video planes        */

    /*
     * Calculate offset of build buffer plane to be mapped into plane 0
     * of display.
     */
    p_off = (3 - (show_x & 3));

    /* Switch to the other target screen in video memory. */
    target_img ^= 0x4000;

    /* Calculate the source address. */
    addr = img3 + (show_x >> 2) + show_y * SCROLL_X_WIDTH;

    /* Draw to each plane in the video memory. */
    for (i = 0; i < 4; i++) {
        SET_WRITE_MASK (1 << (i + 8));
        copy_image (addr + ((p_off - i + 4) & 3) * SCROLL_SIZE + (p_off < i),
                target_img, 16000);
    }

    /*
     * Change the VGA registers to point the top left of the screen
     * to the video memory that we just filled.
     */
    OUTW (0x03D4, (target_img & 0xFF00) | 0x0C);
    OUTW (0x03D4, ((target_img & 0x00FF) << 8) | 0x0D);
}


/*
 * clear_screens
 *   DESCRIPTION: Fills the video memory with zeroes.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: fills all 256kB of VGA video memory with zeroes
 */
    void
clear_screens ()
{
    /* Write to all four planes at once. */
    SET_WRITE_MASK (0x0F00);

    /* Set 64kB to zero (times four planes = 256kB). */
    memset (mem_image, 0, MODE_X_MEM_SIZE);
}


/*
 * The functions inside the preprocessor block below rely on functions
 * in maze.c to generate graphical images of the maze.  These functions
 * are neither available nor necessary for the text restoration program
 * based on this file, and are omitted to simplify linking that program.
 */
#if !defined(TEXT_RESTORE_PROGRAM)


/*
 * draw_vert_line
 *   DESCRIPTION: Draw a vertical map line into the build buffer.  The
 *                line should be offset from the left side of the logical
 *                view window screen by the given number of pixels.
 *   INPUTS: x -- the 0-based pixel column number of the line to be drawn
 *                within the logical view window (equivalent to the number
 *                of pixels from the leftmost pixel to the line to be
 *                drawn)
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success.  If x is outside of the valid
 *                 SCROLL range, the function returns -1.
 *   SIDE EFFECTS: draws into the build buffer
 */
    int
draw_vert_line (int x)
{
    unsigned char buf[SCROLL_Y_DIM]; /* buffer for graphical image of line */
    unsigned char* addr;             /* address of first pixel in build    */
    /*     buffer (without plane offset)  */
    int p_off;                       /* offset of plane of first pixel     */
    int i;			     /* loop index over pixels             */

    /* Check whether requested line falls in the logical view window. */
    if (x < 0 || x >= SCROLL_X_DIM) return -1;

    /* Adjust y to the logical row value. */
    x += show_x;

    /* Get the image of the line. */
    (*vert_line_fn) (x, show_y, buf);

    /* Calculate starting address in build buffer. */
    addr = img3 + (x >> 2) + show_y * SCROLL_X_WIDTH;

    /* Calculate plane offset of first pixel. */
    p_off = (3 - (x & 3));

    /* Copy image data into appropriate planes in build buffer. */
    for (i = 0; i < SCROLL_Y_DIM; i++) {
        addr[i * SCROLL_X_WIDTH + p_off * SCROLL_SIZE] = buf[i];
    }

    /* Return success. */
    return 0;
}


/*
 * draw_horiz_line
 *   DESCRIPTION: Draw a horizontal map line into the build buffer.  The
 *                line should be offset from the top of the logical view
 *                window screen by the given number of pixels.
 *   INPUTS: y -- the 0-based pixel row number of the line to be drawn
 *                within the logical view window (equivalent to the number
 *                of pixels from the top pixel to the line to be drawn)
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success.  If y is outside of the valid
 *                 SCROLL range, the function returns -1.
 *   SIDE EFFECTS: draws into the build buffer
 */
    int
draw_horiz_line (int y)
{
    unsigned char buf[SCROLL_X_DIM]; /* buffer for graphical image of line */
    unsigned char* addr;             /* address of first pixel in build    */
    /*     buffer (without plane offset)  */
    int p_off;                       /* offset of plane of first pixel     */
    int i;			     /* loop index over pixels             */

    /* Check whether requested line falls in the logical view window. */
    if (y < 0 || y >= SCROLL_Y_DIM) return -1;

    /* Adjust y to the logical row value. */
    y += show_y;

    /* Get the image of the line. */
    (*horiz_line_fn) (show_x, y, buf);

    /* Calculate starting address in build buffer. */
    addr = img3 + (show_x >> 2) + y * SCROLL_X_WIDTH;

    /* Calculate plane offset of first pixel. */
    p_off = (3 - (show_x & 3));

    /* Copy image data into appropriate planes in build buffer. */
    for (i = 0; i < SCROLL_X_DIM; i++) {
        addr[p_off * SCROLL_SIZE] = buf[i];
        if (--p_off < 0) {
            p_off = 3;
            addr++;
        }
    }

    /* Return success. */
    return 0;
}

#endif /* !defined(TEXT_RESTORE_PROGRAM) */


/*
 * open_memory_and_ports
 *   DESCRIPTION: Map video memory into our address space; obtain permission
 *                to access VGA ports.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: prints an error message to stdout on failure
 */
    static int
open_memory_and_ports ()
{
    int mem_fd;  /* file descriptor for physical memory image */

    /* Obtain permission to access ports 0x03C0 through 0x03DA. */
    if (ioperm (0x03C0, 0x03DA - 0x03C0 + 1, 1) == -1) {
        perror ("set port permissions");
        return -1;
    }

    /* Open file to access physical memory. */
    if ((mem_fd = open ("/dev/mem", O_RDWR)) == -1) {
        perror ("open /dev/mem");
        return -1;
    }

    /* Map video memory (0xA0000 - 0xBFFFF) into our address space. */
    if ((mem_image = mmap (0, VID_MEM_SIZE, PROT_READ | PROT_WRITE,
                    MAP_SHARED, mem_fd, 0xA0000)) == MAP_FAILED) {
        perror ("mmap video memory");
        return -1;
    }

    /* Close /dev/mem file descriptor and return success. */
    (void)close (mem_fd);
    return 0;
}


/*
 * VGA_blank
 *   DESCRIPTION: Blank or unblank the VGA display.
 *   INPUTS: blank_bit -- set to 1 to blank, 0 to unblank
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
    static void
VGA_blank (int blank_bit)
{
    /*
     * Move blanking bit into position for VGA sequencer register
     * (index 1).
     */
    blank_bit = ((blank_bit & 1) << 5);

    asm volatile (
            "movb $0x01,%%al         /* Set sequencer index to 1. */       ;"
            "movw $0x03C4,%%dx                                             ;"
            "outb %%al,(%%dx)                                              ;"
            "incw %%dx                                                     ;"
            "inb (%%dx),%%al         /* Read old value.           */       ;"
            "andb $0xDF,%%al         /* Calculate new value.      */       ;"
            "orl %0,%%eax                                                  ;"
            "outb %%al,(%%dx)        /* Write new value.          */       ;"
            "movw $0x03DA,%%dx       /* Enable display (0x20->P[0x3C0]) */ ;"
            "inb (%%dx),%%al         /* Set attr reg state to index. */    ;"
            "movw $0x03C0,%%dx       /* Write index 0x20 to enable. */     ;"
            "movb $0x20,%%al                                               ;"
            "outb %%al,(%%dx)                                               "
            : : "g" (blank_bit) : "eax", "edx", "memory");
}


/*
 * set_seq_regs_and_reset
 *   DESCRIPTION: Set VGA sequencer registers and miscellaneous output
 *                register; array of registers should force a reset of
 *                the VGA sequencer, which is restored to normal operation
 *                after a brief delay.
 *   INPUTS: table -- table of sequencer register values to use
 *           val -- value to which miscellaneous output register should be set
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
    static void
set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS],
        unsigned char val)
{
    /*
     * Dump table of values to sequencer registers.  Includes forced reset
     * as well as video blanking.
     */
    REP_OUTSW (0x03C4, table, NUM_SEQUENCER_REGS);

    /* Delay a bit... */
    {volatile int ii; for (ii = 0; ii < 10000; ii++);}

    /* Set VGA miscellaneous output register. */
    OUTB (0x03C2, val);

    /* Turn sequencer on (array values above should always force reset). */
    OUTW (0x03C4,0x0300);
}


/*
 * set_CRTC_registers
 *   DESCRIPTION: Set VGA cathode ray tube controller (CRTC) registers.
 *   INPUTS: table -- table of CRTC register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
    static void
set_CRTC_registers (unsigned short table[NUM_CRTC_REGS])
{
    /* clear protection bit to enable write access to first few registers */
    OUTW (0x03D4, 0x0011);
    REP_OUTSW (0x03D4, table, NUM_CRTC_REGS);
}


/*
 * set_attr_registers
 *   DESCRIPTION: Set VGA attribute registers.  Attribute registers use
 *                a single port and are thus written as a sequence of bytes
 *                rather than a sequence of words.
 *   INPUTS: table -- table of attribute register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
    static void
set_attr_registers (unsigned char table[NUM_ATTR_REGS * 2])
{
    /* Reset attribute register to write index next rather than data. */
    asm volatile (
            "inb (%%dx),%%al"
            : : "d" (0x03DA) : "eax", "memory");
    REP_OUTSB (0x03C0, table, NUM_ATTR_REGS * 2);
}


/*
 * set_graphics_registers
 *   DESCRIPTION: Set VGA graphics registers.
 *   INPUTS: table -- table of graphics register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
    static void
set_graphics_registers (unsigned short table[NUM_GRAPHICS_REGS])
{
    REP_OUTSW (0x03CE, table, NUM_GRAPHICS_REGS);
}

/* sets extra palette */
void set_palette(void *pal)
{
    /* Start writing at color 64. */
    OUTB (0x03C8, 64);

    /* Write all 192 colors from array. */
    REP_OUTSB (0x03C9, pal, (192 * 3));
}

/*
 * fill_palette_mode_x
 *   DESCRIPTION: Fill VGA palette with necessary colors for the adventure
 *                game.  Only the first 64 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 64 palette colors
 */
    static void
fill_palette_mode_x ()
{
    /* 6-bit RGB (red, green, blue) values for first 64 colors */
    /* these are coded for 2 bits red, 2 bits green, 2 bits blue */
    static unsigned char palette_RGB[64][3] = {
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x15},
        {0x00, 0x00, 0x2A}, {0x00, 0x00, 0x3F},
        {0x00, 0x15, 0x00}, {0x00, 0x15, 0x15},
        {0x00, 0x15, 0x2A}, {0x00, 0x15, 0x3F},
        {0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x15},
        {0x00, 0x2A, 0x2A}, {0x00, 0x2A, 0x3F},
        {0x00, 0x3F, 0x00}, {0x00, 0x3F, 0x15},
        {0x00, 0x3F, 0x2A}, {0x00, 0x3F, 0x3F},
        {0x15, 0x00, 0x00}, {0x15, 0x00, 0x15},
        {0x15, 0x00, 0x2A}, {0x15, 0x00, 0x3F},
        {0x15, 0x15, 0x00}, {0x15, 0x15, 0x15},
        {0x15, 0x15, 0x2A}, {0x15, 0x15, 0x3F},
        {0x15, 0x2A, 0x00}, {0x15, 0x2A, 0x15},
        {0x15, 0x2A, 0x2A}, {0x15, 0x2A, 0x3F},
        {0x15, 0x3F, 0x00}, {0x15, 0x3F, 0x15},
        {0x15, 0x3F, 0x2A}, {0x15, 0x3F, 0x3F},
        {0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x15},
        {0x2A, 0x00, 0x2A}, {0x2A, 0x00, 0x3F},
        {0x2A, 0x15, 0x00}, {0x2A, 0x15, 0x15},
        {0x2A, 0x15, 0x2A}, {0x2A, 0x15, 0x3F},
        {0x2A, 0x2A, 0x00}, {0x2A, 0x2A, 0x15},
        {0x2A, 0x2A, 0x2A}, {0x2A, 0x2A, 0x3F},
        {0x2A, 0x3F, 0x00}, {0x2A, 0x3F, 0x15},
        {0x2A, 0x3F, 0x2A}, {0x2A, 0x3F, 0x3F},
        {0x3F, 0x00, 0x00}, {0x3F, 0x00, 0x15},
        {0x3F, 0x00, 0x2A}, {0x3F, 0x00, 0x3F},
        {0x3F, 0x15, 0x00}, {0x3F, 0x15, 0x15},
        {0x3F, 0x15, 0x2A}, {0x3F, 0x15, 0x3F},
        {0x3F, 0x2A, 0x00}, {0x3F, 0x2A, 0x15},
        {0x3F, 0x2A, 0x2A}, {0x3F, 0x2A, 0x3F},
        {0x3F, 0x3F, 0x00}, {0x3F, 0x3F, 0x15},
        {0x3F, 0x3F, 0x2A}, {0x3F, 0x3F, 0x3F}
    };

    /* Start writing at color 0. */
    OUTB (0x03C8, 0x00);

    /* Write all 64 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB, 64 * 3);
}


/*
 * fill_palette_text
 *   DESCRIPTION: Fill VGA palette with default VGA colors.
 *                Only the first 32 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 32 palette colors
 */
    static void
fill_palette_text ()
{
    /* 6-bit RGB (red, green, blue) values VGA colors and grey scale */
    static unsigned char palette_RGB[32][3] = {
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x2A},   /* palette 0x00 - 0x0F    */
        {0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x2A},   /* basic VGA colors       */
        {0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x2A},
        {0x2A, 0x15, 0x00}, {0x2A, 0x2A, 0x2A},
        {0x15, 0x15, 0x15}, {0x15, 0x15, 0x3F},
        {0x15, 0x3F, 0x15}, {0x15, 0x3F, 0x3F},
        {0x3F, 0x15, 0x15}, {0x3F, 0x15, 0x3F},
        {0x3F, 0x3F, 0x15}, {0x3F, 0x3F, 0x3F},
        {0x00, 0x00, 0x00}, {0x05, 0x05, 0x05},   /* palette 0x10 - 0x1F    */
        {0x08, 0x08, 0x08}, {0x0B, 0x0B, 0x0B},   /* VGA grey scale         */
        {0x0E, 0x0E, 0x0E}, {0x11, 0x11, 0x11},
        {0x14, 0x14, 0x14}, {0x18, 0x18, 0x18},
        {0x1C, 0x1C, 0x1C}, {0x20, 0x20, 0x20},
        {0x24, 0x24, 0x24}, {0x28, 0x28, 0x28},
        {0x2D, 0x2D, 0x2D}, {0x32, 0x32, 0x32},
        {0x38, 0x38, 0x38}, {0x3F, 0x3F, 0x3F}
    };

    /* Start writing at color 0. */
    OUTB (0x03C8, 0x00);

    /* Write all 32 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB, 32 * 3);
}


/*
 * write_font_data
 *   DESCRIPTION: Copy font data into VGA memory, changing and restoring
 *                VGA register values in order to do so.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: leaves VGA registers in final text mode state
 */
    static void
write_font_data ()
{
    int i;                /* loop index over characters                   */
    int j;                /* loop index over font bytes within characters */
    unsigned char* fonts; /* pointer into video memory                    */

    /* Prepare VGA to write font data into video memory. */
    OUTW (0x3C4, 0x0402);
    OUTW (0x3C4, 0x0704);
    OUTW (0x3CE, 0x0005);
    OUTW (0x3CE, 0x0406);
    OUTW (0x3CE, 0x0204);

    /* Copy font data from array into video memory. */
    for (i = 0, fonts = mem_image; i < 256; i++) {
        for (j = 0; j < 16; j++)
            fonts[j] = font_data[i][j];
        fonts += 32; /* skip 16 bytes between characters */
    }

    /* Prepare VGA for text mode. */
    OUTW (0x3C4, 0x0302);
    OUTW (0x3C4, 0x0304);
    OUTW (0x3CE, 0x1005);
    OUTW (0x3CE, 0x0E06);
    OUTW (0x3CE, 0x0004);
}


/*
 * set_text_mode_3
 *   DESCRIPTION: Put VGA into text mode 3 (color text).
 *   INPUTS: clear_scr -- if non-zero, clear screens; otherwise, do not
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: may clear screens; writes font data to video memory
 */
    static void
set_text_mode_3 (int clear_scr)
{
    unsigned long* txt_scr; /* pointer to text screens in video memory */
    int i;                  /* loop over text screen words             */

    VGA_blank (1);                               /* blank the screen        */
    /*
     * The value here had been changed to 0x63, but seems to work
     * fine in QEMU (and VirtualPC, where I got it) with the 0x04
     * bit set (VGA_MIS_DCLK_28322_720).
     */
    set_seq_regs_and_reset (text_seq, 0x67);     /* sequencer registers     */
    set_CRTC_registers (text_CRTC);              /* CRT control registers   */
    set_attr_registers (text_attr);              /* attribute registers     */
    set_graphics_registers (text_graphics);      /* graphics registers      */
    fill_palette_text ();			 /* palette colors          */
    if (clear_scr) {				 /* clear screens if needed */
        txt_scr = (unsigned long*)(mem_image + 0x18000);
        for (i = 0; i < 8192; i++)
            *txt_scr++ = 0x07200720;
    }
    write_font_data ();                          /* copy fonts to video mem */
    VGA_blank (0);			         /* unblank the screen      */
}


/*
 * copy_image
 *   DESCRIPTION: Copy one plane of a screen from the build buffer to the
 *                video memory.
 *   INPUTS: img -- a pointer to a single screen plane in the build buffer
 *           scr_addr -- the destination offset in video memory
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies a plane from the build buffer to video memory
 */
    static void
copy_image (unsigned char* img, unsigned short scr_addr, unsigned short num)
{
    /*
     * memcpy is actually probably good enough here, and is usually
     * implemented using ISA-specific features like those below,
     * but the code here provides an example of x86 string moves
     */
    asm volatile (
            "cld                                                 ;"
            "movl %%ebx, %%ecx                                  ;"
            "rep movsb    # copy ECX bytes from M[ESI] to M[EDI]  "
            : /* no outputs */
            : "b" (num), "S" (img), "D" (mem_image + scr_addr)
            : "eax", "ecx", "memory"
            );
}


#if defined(TEXT_RESTORE_PROGRAM)

/*
 * main -- for the "tr" program
 *   DESCRIPTION: Put the VGA into text mode 3 without clearing the screens,
 *                which serves as a useful debugging tool when trying to
 *                debug programs that rely on having the VGA in mode X for
 *                normal operation.  Writes font data to video memory.
 *   INPUTS: none (command line arguments are ignored)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, 3 in panic scenarios
 */
    int
main ()
{
    /* Map video memory and obtain permission for VGA port access. */
    if (open_memory_and_ports () == -1)
        return 3;

    /* Put VGA into text mode without clearing the screen. */
    set_text_mode_3 (0);

    /* Unmap video memory. */
    (void)munmap (mem_image, VID_MEM_SIZE);

    /* Return success. */
    return 0;
}

#endif
