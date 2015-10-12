/*									tab:8
 *
 * photo.c - photo display functions
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
 * Version:	    3
 * Creation Date:   Fri Sep  9 21:44:10 2011
 * Filename:	    photo.c
 * History:
 *	SL	1	Fri Sep  9 21:44:10 2011
 *		First written (based on mazegame code).
 *	SL	2	Sun Sep 11 14:57:59 2011
 *		Completed initial implementation of functions.
 *	SL	3	Wed Sep 14 21:49:44 2011
 *		Cleaned up code for distribution.
 */


#include <string.h>

#include "assert.h"
#include "modex.h"
#include "photo.h"
#include "photo_headers.h"
#include "world.h"


/* types local to this file (declared in types.h) */

/*
 * A room photo.  Note that you must write the code that selects the
 * optimized palette colors and fills in the pixel data using them as
 * well as the code that sets up the VGA to make use of these colors.
 * Pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of
 * the second row, and so forth.  No padding should be used.
 */
struct photo_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t        palette[192][3];     /* optimized palette colors */
    uint8_t*       img;                 /* pixel data               */
};

/*
 * An object image.  The code for managing these images has been given
 * to you.  The data are simply loaded from a file, where they have
 * been stored as 2:2:2-bit RGB values (one byte each), including
 * transparent pixels (value OBJ_CLR_TRANSP).  As with the room photos,
 * pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of the
 * second row, and so forth.  No padding is used.
 */
struct image_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t*       img;                 /* pixel data               */
};

typedef struct node {
    uint32_t total_red;
    uint32_t total_green;
    uint32_t total_blue;
    int index;
    int counter;
    int popular;
} node_t;

node_t octree_lev2[64];
node_t octree_lev4[4096];
int indices_lev4[4096];


void init_octree(photo_t* p, int* image);
int node_compare(const void *a, const void *b);
void popular_color(photo_t* p);
void yield_average_color(node_t* node, uint8_t color[3]);
void generate_palette(photo_t* p, int* image);

/* file-scope variables */

/*
 * The room currently shown on the screen.  This value is not known to
 * the mode X code, but is needed when filling buffers in callbacks from
 * that code (fill_horiz_buffer/fill_vert_buffer).  The value is set
 * by calling prep_room.
 */
static const room_t* cur_room = NULL;


/*
 * fill_horiz_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the leftmost
 *                pixel of a line to be drawn on the screen, this routine
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- leftmost pixel of line to be drawn
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM])
{
    int            idx;   /* loop index over pixels in the line          */
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgx;  /* loop index over pixels in object image      */
    int            yoff;  /* y offset into object image                  */
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; idx++) {
        buf[idx] = (0 <= x + idx && view->hdr.width > x + idx ?
		    view->img[view->hdr.width * y + x + idx] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (y < obj_y || y >= obj_y + img->hdr.height ||
	    x + SCROLL_X_DIM <= obj_x || x >= obj_x + img->hdr.width) {
	    continue;
	}

	/* The y offset of drawing is fixed. */
	yoff = (y - obj_y) * img->hdr.width;

	/*
	 * The x offsets depend on whether the object starts to the left
	 * or to the right of the starting point for the line being drawn.
	 */
	if (x <= obj_x) {
	    idx = obj_x - x;
	    imgx = 0;
	} else {
	    idx = 0;
	    imgx = x - obj_x;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_X_DIM > idx && img->hdr.width > imgx; idx++, imgx++) {
	    pixel = img->img[yoff + imgx];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/*
 * fill_vert_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the top pixel of
 *                a vertical line to be drawn on the screen, this routine
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- top pixel of line to be drawn
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM])
{
    int            idx;   /* loop index over pixels in the line          */
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgy;  /* loop index over pixels in object image      */
    int            xoff;  /* x offset into object image                  */
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; idx++) {
        buf[idx] = (0 <= y + idx && view->hdr.height > y + idx ?
		    view->img[view->hdr.width * (y + idx) + x] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (x < obj_x || x >= obj_x + img->hdr.width ||
	    y + SCROLL_Y_DIM <= obj_y || y >= obj_y + img->hdr.height) {
	    continue;
	}

	/* The x offset of drawing is fixed. */
	xoff = x - obj_x;

	/*
	 * The y offsets depend on whether the object starts below or
	 * above the starting point for the line being drawn.
	 */
	if (y <= obj_y) {
	    idx = obj_y - y;
	    imgy = 0;
	} else {
	    idx = 0;
	    imgy = y - obj_y;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_Y_DIM > idx && img->hdr.height > imgy; idx++, imgy++) {
	    pixel = img->img[xoff + img->hdr.width * imgy];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/*
 * image_height
 *   DESCRIPTION: Get height of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t
image_height (const image_t* im)
{
    return im->hdr.height;
}


/*
 * image_width
 *   DESCRIPTION: Get width of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t
image_width (const image_t* im)
{
    return im->hdr.width;
}

/*
 * photo_height
 *   DESCRIPTION: Get height of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t
photo_height (const photo_t* p)
{
    return p->hdr.height;
}


/*
 * photo_width
 *   DESCRIPTION: Get width of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t
photo_width (const photo_t* p)
{
    return p->hdr.width;
}


/*
 * prep_room
 *   DESCRIPTION: Prepare a new room for display.  You might want to set
 *                up the VGA palette registers according to the color
 *                palette that you chose for this room.
 *   INPUTS: r -- pointer to the new room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes recorded cur_room for this file
 */
void
prep_room (const room_t* r)
{
    /* Record the current room. */
    cur_room = r;
    set_palette(get_room_view(cur_room)->palette);
}


/*
 * read_obj_image
 *   DESCRIPTION: Read size and pixel data in 2:2:2 RGB format from a
 *                photo file and create an image structure from it.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the image
 */
image_t*
read_obj_image (const char* fname)
{
    FILE*    in;		/* input file               */
    image_t* img = NULL;	/* image structure          */
    uint16_t x;			/* index over image columns */
    uint16_t y;			/* index over image rows    */
    uint8_t  pixel;		/* one pixel from the file  */

    /*
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the image pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (img = malloc (sizeof (*img))) ||
	NULL != (img->img = NULL) || /* false clause for initialization */
	1 != fread (&img->hdr, sizeof (img->hdr), 1, in) ||
	MAX_OBJECT_WIDTH < img->hdr.width ||
	MAX_OBJECT_HEIGHT < img->hdr.height ||
	NULL == (img->img = malloc
		 (img->hdr.width * img->hdr.height * sizeof (img->img[0])))) {
	if (NULL != img) {
	    if (NULL != img->img) {
	        free (img->img);
	    }
	    free (img);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

    /*
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = img->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
	for (x = 0; img->hdr.width > x; x++) {

	    /*
	     * Try to read one 8-bit pixel.  On failure, clean up and
	     * return NULL.
	     */
	    if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
		free (img->img);
		free (img);
	        (void)fclose (in);
		return NULL;
	    }

	    /* Store the pixel in the image data. */
	    img->img[img->hdr.width * y + x] = pixel;
	}
    }

    /* All done.  Return success. */
    (void)fclose (in);
    return img;
}

/* quick function of getting level 2 index in octree */
int get_lev2_idx(int pixel) {
    return ((((pixel >> 14) & 0x3) << 4) |
         (((pixel >> 9) & 0x3) << 2) |
         ((pixel >> 3) & 0x3));
}

/* quick function of getting level 4 index in octree */
int get_lev4_idx(int pixel) {
    return ((((pixel >> 12) & 0xf) << 8) |
         (((pixel >> 7) & 0xF) << 4) |
         ((pixel >> 1) & 0xF));
}

/* quick function of getting red color in a pixel */
uint8_t get_red_color(int pixel) {
     return pixel >> 11;
}

/* quick function of getting green color in a pixel */
uint8_t get_green_color(int pixel) {
     return (pixel >> 5) & 0x3F;
}

/* quick function of getting blue color in a pixel */
uint8_t get_blue_color(int pixel) {
     return pixel & 0x1F;
}

/*
 * read_photo
 *   DESCRIPTION: Read size and pixel data in 5:6:5 RGB format from a
 *                photo file and create a photo structure from it.
 *                Code provided simply maps to 2:2:2 RGB.  You must
 *                replace this code with palette color selection, and
 *                must map the image pixels into the palette colors that
 *                you have defined.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the photo
 */
photo_t*
read_photo (const char* fname)
{
    FILE*    in;	/* input file               */
    photo_t* p = NULL;	/* photo structure          */
    uint16_t x;		/* index over image columns */
    uint16_t y;		/* index over image rows    */
    uint16_t pixel;	/* one pixel from the file  */

    /*
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the photo pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (p = malloc (sizeof (*p))) ||
	NULL != (p->img = NULL) || /* false clause for initialization */
	1 != fread (&p->hdr, sizeof (p->hdr), 1, in) ||
	MAX_PHOTO_WIDTH < p->hdr.width ||
	MAX_PHOTO_HEIGHT < p->hdr.height ||
	NULL == (p->img = malloc
		 (p->hdr.width * p->hdr.height * sizeof (p->img[0])))) {
	if (NULL != p) {
	    if (NULL != p->img) {
	        free (p->img);
	    }
	    free (p);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

    int *image = (int*) malloc(p->hdr.width * p->hdr.height * sizeof(int));
    if (image == NULL) return NULL;

    /*
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = p->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
	for (x = 0; p->hdr.width > x; x++) {

	    /*
	     * Try to read one 16-bit pixel.  On failure, clean up and
	     * return NULL.
	     */
	    if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
		free (p->img);
		free (p);
	        (void)fclose (in);
		return NULL;

	    }
	    /*
	     * 16-bit pixel is coded as 5:6:5 RGB (5 bits red, 6 bits green,
	     * and 6 bits blue).  We change to 2:2:2, which we've set for the
	     * game objects.  You need to use the other 192 palette colors
	     * to specialize the appearance of each photo.
	     *
	     * In this code, you need to calculate the p->palette values,
	     * which encode 6-bit RGB as arrays of three uint8_t's.  When
	     * the game puts up a photo, you should then change the palette
	     * to match the colors needed for that photo.
	     */
        image[x + p->hdr.width * y] = pixel;
	}
    }

    /* init the octree */
    init_octree(p, image);

    /* quick sort the popular node in octree level 4 */
    qsort(indices_lev4, 4096, sizeof(int), node_compare);

    /* set popular color in photo */
    popular_color(p);

    /* generate palette to the photo accroding to the popular color */
    generate_palette(p, image);

    /* free the dynamically allocated variable */
    free(image);

    /* All done.  Return success. */
    (void)fclose (in);
    return p;
}


/*
 * init_octree
 *   DESCRIPTION: Initialize all octree variable, including 2 and 4 levels
 *   INPUTS: p - entire photo information
 *          image - pixels info in the photo
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: set octree_lev2 and octree_lev4
 */
void init_octree(photo_t* p, int* image) {
    int x, y, index_lev2, index_lev4, pixel;
    uint8_t red, green, blue;

    /* init all to 0 */
    memset(octree_lev2, 0, 64 * sizeof(node_t));
    memset(octree_lev4, 0, 4096 * sizeof(node_t));

    /* loop through every pixel */
    for (y = 0; y < p->hdr.height; y++) {
        for (x = 0; x < p->hdr.width; x++) {
            /* current pixel point */

            pixel = image[x + p->hdr.width * y];
            /* indexs of 2th and 4th octree */
            index_lev2 = get_lev2_idx(pixel);
            index_lev4 = get_lev4_idx(pixel);

            /* increment node counter */
            octree_lev2[index_lev2].counter++;
            octree_lev4[index_lev4].counter++;

            /* get current color info of pixel */
            red = get_red_color(pixel);
            green = get_green_color(pixel);
            blue = get_blue_color(pixel);

            /* update cumulative color of 2th and 4th color, rgb */
            octree_lev2[index_lev2].total_red += red;
            octree_lev2[index_lev2].total_green += green;
            octree_lev2[index_lev2].total_blue += blue;
            octree_lev4[index_lev4].total_red += red;
            octree_lev4[index_lev4].total_green += green;
            octree_lev4[index_lev4].total_blue += blue;
        }
    }

    for (x = 0; x < 4096; x++) indices_lev4[x] = x;
}


/*
 * node_compare
 *   DESCRIPTION: function pointer of quick sort comparing
 *   INPUTS: a - first index of octree
 *           b - second index of octree
 *   OUTPUTS: -1 for a > b, 0 for equal, 1 for a < b
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int node_compare(const void *a, const void *b) {
    /* cast the pointer */
    int *index_a = (int*)a;
    int *index_b = (int*)b;

    /* compare the counter in a index and b index of octree */
    if (octree_lev4[*index_a].counter > octree_lev4[*index_b].counter) return -1;
    else if (octree_lev4[*index_a].counter == octree_lev4[*index_b].counter) return 0;
    else return 1;
}

/*
 * popular_color
 *   DESCRIPTION: get 128 most popular color from level 4 in octree
 *   INPUTS: p - info of the photo
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: change a yield of first 128 node in 4 level of octree
 */
void popular_color(photo_t* p) {
    int i;
    /* loop throught first 128 node in 4th level of octree */
    for (i = 0; i < 128; i++) {
        /* set to puoular node */
        octree_lev4[indices_lev4[i]].popular = 1;
        octree_lev4[indices_lev4[i]].index = i;
        /* calculate average color mapping */
        yield_average_color(&octree_lev4[indices_lev4[i]], p->palette[i+64]);
    }
}

/*
 * round_color
 *   DESCRIPTION: round the number and return a uint8_t
 *   INPUTS: num - a float pointer number
 *   OUTPUTS: uint8_t
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
uint8_t round_color(float num) {
    /* cast float to int */
    int intn = (int)num;
    if ((intn + 0.5) >= num) return intn + 1;
    else return intn;
}

/*
 * yield_average_color
 *   DESCRIPTION: find out cumulative color's average
 *   INPUTS: node - a node in octree
 *           color[3] - rgb color array pointer
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: color pointer will be changed
 */
void yield_average_color(node_t* node, uint8_t color[3]) {
    float counter = node->counter;
    /* field bit mapping : rrrrrggggggbbbbb, so shift */
    color[0] = round_color((node->total_red << 1) / counter);
    color[1] = round_color(node->total_green /  counter);
    color[2] = round_color((node->total_blue << 1) / counter);
}

/*
 * generate_palette
 *   DESCRIPTION: generate palette according to all calculation above
 *   INPUTS: p - entire photo information
 *          image - pixels info in the photo
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: photo palette would be changed accordingly
 */
void generate_palette(photo_t* p, int* image) {
    /* local variables */
    int x, y, index_lev2, index_lev4, pixel;
    uint8_t red, green, blue;

    /* loop through every pixel */
    for (y = 0; y < p->hdr.height; y++) {
        for (x = 0; x < p->hdr.width; x++) {
            /* current pixel in the image */
            pixel = image[x + y * p->hdr.width];
            /* get 2th and 4th index of octree */
            index_lev2 = get_lev2_idx(pixel);
            index_lev4 = get_lev4_idx(pixel);

            /* handle if the node is popular */
            if (octree_lev4[index_lev4].popular) {
                red = get_red_color(pixel);
                green = get_green_color(pixel);
                blue = get_blue_color(pixel);

                /* same pixel only once, so delete in level 2*/
                octree_lev2[index_lev2].total_red -= red;
                octree_lev2[index_lev2].total_green -= green;
                octree_lev2[index_lev2].total_blue -= blue;
                octree_lev2[index_lev2].counter--;

                /* set photo's color */
                p->img[x + p->hdr.width * y] = octree_lev4[index_lev4].index + 128;
            }
        }
    }

    /* loop through every pixel */
    for (y = 0; y < p->hdr.height; y++) {
        for (x = 0; x < p->hdr.width; x++) {
            /* current pixel in the image */
            pixel = image[x + y * p->hdr.width];
            /* get 2th and 4th index of octree */
            index_lev2 = get_lev2_idx(pixel);
            index_lev4 = get_lev4_idx(pixel);

            if (octree_lev4[index_lev4].popular == 0) {
                /* set average color of current palette */
                yield_average_color(&octree_lev2[index_lev2], p->palette[index_lev2]);
                /* skip 64 because of the size of 2th level of octree */
                p->img[x + p->hdr.width * y] = index_lev2 + 64;
            }
        }
    }
}
