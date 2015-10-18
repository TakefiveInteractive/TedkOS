#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

uint32_t ece391_strlen(const uint8_t* s)
{
    uint32_t len;

    for (len = 0; '\0' != *s; s++, len++);
    return len;
}

void ece391_strcpy(uint8_t* dst, const uint8_t* src)
{
    while ('\0' != (*dst++ = *src++));
}

void ece391_fdputs(int32_t fd, const uint8_t* s)
{
    (void)ece391_write (fd, s, ece391_strlen(s));
}

int32_t ece391_strcmp(const uint8_t* s1, const uint8_t* s2)
{
    while (*s1 == *s2) {
        if (*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

int32_t ece391_strncmp(const uint8_t* s1, const uint8_t* s2, uint32_t n)
{
    if (0 == n)
        return 0;
    while (*s1 == *s2) {
        if (*s1 == '\0' || --n == 0)
        return 0;
    s1++;
    s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

/* Convert a number to its ASCII representation, with base "radix" */
uint8_t* ece391_itoa(uint32_t value, uint8_t* buf, int32_t radix)
{
        static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        uint8_t *newbuf = buf;
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
        while (newval > 0) {
                i = newval % radix;
                *newbuf = lookup[i];
                newbuf++;
                newval /= radix;
        }

        /* Add a terminating NULL */
        *newbuf = '\0';

        /* Reverse the string and return */
        return ece391_strrev(buf);
}

/* In-place string reversal */
uint8_t* ece391_strrev(uint8_t* s)
{
    register uint8_t tmp;
    register int32_t beg = 0;
    register int32_t end = ece391_strlen(s) - 1;

    if (end <= 0) {
        return s;
    }

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
   }

   return s;
}

