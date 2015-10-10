/*									tab:8
 *
 * photo_headers.h - header file defining image file header format structures
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
 * DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
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
 * Creation Date:   Wed Sep 14 10:08:14 2011
 * Filename:	    photo_headers.h
 * History:
 *	SL	1	Wed Sep 14 10:08:14 2011
 *		Extracted from mp2photo.c for unification purposes.
 */

#if !defined(PHOTO_HEADERS_H)
#define PHOTO_HEADERS_H


#include <stdint.h>


#define BMP_MAGIC      "BM"	/* BMP file magic sequence (before header) */

#define OBJ_CLR_TRANSP 0x40	/* transparent pixel color in object image */


/* 
 * BMP header.  This structure is deliberately incomplete: it allows code
 * to do a few sanity checks and to find the pixel data.
 */
typedef struct bmp_header_t bmp_header_t;
struct bmp_header_t {
    uint32_t file_size;
    uint32_t reserved;
    uint32_t pixel_offset;
    /* DIB header follows... */
    uint32_t dib_header_size;
    uint32_t img_width;
    uint32_t img_height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression_type;
    uint32_t img_size;
    /* We ignore the rest ... */
};

/*
 * Room photo/object image file header for the ECE391 adventure 
 * game (F11 MP2).
 *
 * Room photo pixels are stored as 5:6:5-bit RGB, starting from the 
 * lower left of the image (as with BMP files) scanning across the row 
 * to the right, and then proceeding upwards with subsequent rows.  No 
 * padding is used.
 * 
 * Object image pixels are stored as 2:2:2-bit RGB, with transparent
 * pixels coded as value 0x40.
 */
typedef struct photo_header_t photo_header_t;
struct photo_header_t {
    uint16_t width;	/* image width in pixels  */
    uint16_t height;	/* image height in pixels */
};

#endif /* PHOTO_HEADERS_H */

