#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"


uint32_t
ece391_strlen (const uint8_t* s)
{
    uint32_t len;

    for (len = 0; '\0' != *s; s++, len++);
    return len;
}

void
ece391_strcpy (uint8_t* dst, const uint8_t* src)
{
    while ('\0' != (*dst++ = *src++));
}

void
ece391_fdputs (int32_t fd, const uint8_t* s)
{
    (void)ece391_write (fd, s, ece391_strlen (s));
}

int32_t
ece391_strcmp (const uint8_t* s1, const uint8_t* s2)
{
    while (*s1 == *s2) {
        if (*s1 == '\0')
	    return 0;
	s1++;
	s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

int32_t
ece391_strncmp (const uint8_t* s1, const uint8_t* s2, uint32_t n)
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

