#include <stdint.h>
#include <stddef.h>

#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

// keyboard buffer size
#define BUFSIZE 128

// approximately one page of screen can have 256 chars
#define APPROX_PAGE_SIZE   256

#define termputarr(FD, ARR) {fs_write((FD), (const void*)(ARR), sizeof(ARR)/sizeof(char));}


/*
* int8_t* user_strrev(int8_t* s);
*   Inputs: int8_t* s = string to reverse
*   Return Value: reversed string
*	Function: reverses a string s
*/

char*
user_strrev(char* s)
{
	register char tmp;
	register int32_t beg=0;
	register int32_t end=strlen(s) - 1;

	while(beg < end) {
		tmp = s[end];
		s[end] = s[beg];
		s[beg] = tmp;
		beg++;
		end--;
	}

	return s;
}


/*
* reference: klibs/lib.c
* user side itoa
* int8_t* user_itoa(uint32_t value, int8_t* buf, int32_t radix);
*   Inputs: uint32_t value = number to convert
*			int8_t* buf = allocated buffer to place string in
*			int32_t radix = base system. hex, oct, dec, etc.
*   Return Value: number of bytes written
*	Function: Convert a number to its ASCII representation, with base "radix"
*/

char*
user_itoa(uint32_t value, char* buf, int32_t radix)
{
	static char lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	char *newbuf = buf;
	int32_t i;
	uint32_t newval = value;

	/* Special case for zero */
	if(value == 0) {
		buf[0]='0';
		buf[1]='\0';
		return buf;
	}

	/* Go through the number one place value at a time, and add the
	 * correct digit to "newbuf".  We actually add characters to the
	 * ASCII string from lowest place value to highest, which is the
	 * opposite of how the number should be printed.  We'll reverse the
	 * characters later. */
	while(newval > 0) {
		i = newval % radix;
		*newbuf = lookup[i];
		newbuf++;
		newval /= radix;
	}

	/* Add a terminating NULL */
	*newbuf = '\0';

	/* Reverse the string and return */
	return user_strrev(buf);
}

// Currently new "instance" only needs stdin and stdout
int kreader_main (int32_t term, int32_t kb)
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    termputarr (term, "Starting Kernel CMD: File Reader\n");

    while (1) {
        struct dentry_t dentry;
        uint8_t filebuf[APPROX_PAGE_SIZE] = {};

        termputarr (term, "read file> ");
        if (-1 == (cnt = fs_read (kb, (void*)buf, BUFSIZE-1))) {
            termputarr (term, "read from keyboard failed\n");
            return 3;
        }
        if (cnt > 0 && '\n' == buf[cnt - 1])
            cnt--;
        buf[cnt] = '\0';
        if ('\0' == buf[0])
            continue;

        int fd = fs_open((const char *)buf);
        if (-1 == fd)
        {
            termputarr (term, "no such file\n");
        }
        else
        {
            // 20 chars are enough for 2^64
            #define lenOfStrOfInt 21
            char strOfInt[lenOfStrOfInt];
            int totalLen = 0;
            int len = 0;
            do
            {
                len = fs_read(fd, filebuf, sizeof(filebuf) - 1);
                fs_write(term, (const void*)filebuf, len);
                if (len > 0)
                {
                    termputarr(term, "\n\npress enter to read next block...\n");
                    totalLen += len;
                    fs_read (kb, (void*)buf, 1);
                }
            } while (len > 0);
            termputarr(term, "\n\n Total size: ");
            user_itoa(totalLen, strOfInt, 10);
            fs_write(term, strOfInt, lenOfStrOfInt);
            termputarr(term, "\n\n");
        }
    }
    return 0;
}
