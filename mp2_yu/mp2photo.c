/*									tab:8
 *
 * mp2photo.c - utility program for producing adventure game room photos
 *              and object images
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
 * Version:	    2
 * Creation Date:   Sat Sep 10 00:50:10 2011
 * Filename:	    mp2photo.c
 * History:
 *	SL	1	Sat Sep 10 00:50:10 2011
 *		First written.
 *	SL	2	Sat Sep 14 10:26:34 2011
 *		Merged with mp2object.c.
 */


/* 
 * This file is a standalone utility program that transforms 24-bit BMP
 * files into room photos for the Fall 2011 ECE391 MP2 adventure game.
 * 
 * The input file format is fairly constrained--no compression is allowed,
 * for example.
 *
 * The output file format is 5:6:5 RGB stored in the same order as in the
 * BMP, i.e., rows from bottom to top, and from right to left within each
 * row.  The header simply gives the dimensions of the image.
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "photo_headers.h"


#if !defined(WRITE_OBJECT_IMAGE)
#define WRITE_OBJECT_IMAGE 0		/* output defaults to room photo */
#endif


/* 
 * Calculate width of one row of a BMP image in bytes, including padding
 * (to multiple of 4 bytes).
 */
static int32_t
bmp_row_width (const bmp_header_t* h)
{
    return 4 * ((3 * h->img_width + 3) / 4);
}

// Reads BMP header from file and checks its validity.
// Returns 1 if BMP header is valid, otherwise 0.
static int
bmp_header_check (const char* fname, FILE* in, bmp_header_t* h)
{
    char     magic[3];
    uint32_t row_width;

    // Check validity of input file.
    magic[2] = '\0';
    if (2 != fread (magic, sizeof (magic[0]), 2, in) ||
        0 != strcmp (magic, BMP_MAGIC) ||
	1 != fread (h, sizeof (*h), 1, in)) {
        fprintf (stderr, "%s does not appear to be a BMP file.\n", fname);
	return 0;
    }
    if (4096 < h->img_width || 4096 < h->img_height || 1 != h->planes || 
    	24 != h->bits_per_pixel || 0 != h->compression_type) {
        fprintf (stderr, "%s must be 24-bit-color on one plane with no "
		 "compression.\n", fname);
        return 0;
    }
    row_width = bmp_row_width (h);
    if (h->img_size != row_width * h->img_height) {
        fprintf (stderr, "%s image size incorrect in BMP/DIB header.\n",
		 fname);
        return 0;
    }
    return 1;
}

// Read image data from BMP file into dynamically allocated memory.
// Return pointer to memory on success, or NULL on failure.
static uint8_t*
read_bmp_image_data (FILE* in, const bmp_header_t* h)
{
    uint8_t* img_data;

    // Seek to image data.
    if (0 != fseek (in, h->pixel_offset, SEEK_SET)) {
        perror ("fseek to start of image data in BMP file");
        return NULL;
    }

    // Allocate space, then read in the image data.
    if (NULL == (img_data = malloc (h->img_size)) ||
    	1 != fread (img_data, h->img_size, 1, in)) {
	if (NULL != img_data) {
	    free (img_data);
	}
        perror ("allocate and read image");
	return NULL;
    }
    return img_data;
}

// Write header and data as either 5:6:5 RGB words (little endian) or
// 2:2:2 RGB bytes, row by row, to the output file.  Return 1 on success, 
// 0 on failure.
static int
write_output_file (FILE* out, const bmp_header_t* h, const uint8_t* img)
{
    photo_header_t photo_header;
    uint32_t       row_width;
    uint16_t	   x;
    uint16_t	   y;

    // Write header to output file.
    photo_header.width = h->img_width;
    photo_header.height = h->img_height;
    if (1 != fwrite (&photo_header, sizeof (photo_header), 1, out)) {
        perror ("write header to output file");
	return 0;
    }

    // Write image data to output file.
    row_width = bmp_row_width (h);
    for (y = 0; h->img_height > y; y++) {
	for (x = 0; h->img_width > x; x++) {
#if (1 == WRITE_OBJECT_IMAGE)
	    uint8_t vga_color;
 	    vga_color = ((img[row_width * y + 3 * x + 2] >> 6) << 4) | 
 	    		((img[row_width * y + 3 * x + 1] >> 6) << 2) | 
 			(img[row_width * y + 3 * x] >> 6);
	    /* 
	     * We map any bright yellow pixel to transparent; it's easy to
	     * be more specific by conditioning on the img data (24 bits)
	     * rather than the output image data (6 bits).
	     */
	    if (0x3C == vga_color) {
 	        vga_color = OBJ_CLR_TRANSP;
 	    }
#else /* (1 != WRITE_OBJECT_IMAGE) */
	    uint16_t vga_color;
	    vga_color = ((img[row_width * y + 3 * x + 2] >> 3) << 11) | 
	    		((img[row_width * y + 3 * x + 1] >> 2) << 5) | 
			(img[row_width * y + 3 * x] >> 3);
#endif /* WRITE_OBJECT_IMAGE */
	    if (1 != fwrite (&vga_color, sizeof (vga_color), 1, out)) {
	        perror ("write data to output file");
		return 0;
	    }
	}
    }

    return 1;
}

int
main (int argc, char* argv[])
{
    FILE*        in;
    FILE*        out;
    bmp_header_t bmp_header;
    uint8_t*     img_data;
    int32_t      written;

    // Check syntax of invocation.
    if (3 != argc) {
    	fprintf (stderr, "usage: %s <BMP file name> <output file>\n", argv[0]);
	return 2;
    }

    // Try to open the two files.
    if (NULL == (in = fopen (argv[1], "r+b"))) {
        perror ("open BMP file");
	return 2;
    }
    if (NULL == (out = fopen (argv[2], "w+b"))) {
	fclose (in);
        perror ("open output file");
	return 2;
    }

    // Check validity of input file, then read image data from input file.
    if (!bmp_header_check (argv[1], in, &bmp_header) ||
	NULL == (img_data = read_bmp_image_data (in, &bmp_header))) {
	fclose (in);
	fclose (out);
	return 2;
    }

    // Done with the input file.  Ignore remaining errors.
    (void)fclose (in);

    // Try to write, then close, the output file.
    written = write_output_file (out, &bmp_header, img_data);
    if (EOF == fclose (out)) {
	perror ("close output file");
        written = 0;
    }

    // Free the image data.
    free (img_data);

    // Return value based on success of output file write and close.
    return (written ? 0 : 3);
}

