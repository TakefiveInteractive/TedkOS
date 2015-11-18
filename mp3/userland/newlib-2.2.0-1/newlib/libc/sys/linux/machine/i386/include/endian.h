/*
 * Copyright (c) 1987, 1991 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      from: @(#)endian.h      7.8 (Berkeley) 4/3/91
 *      $Id: endian.h,v 1.2 2008/05/22 21:30:27 jjohnstn Exp $
 */

#ifndef _MACHINE_ENDIAN_H_
#define _MACHINE_ENDIAN_H_

/*
 * Define the order of 32-bit words in 64-bit words.
 */
#define _QUAD_HIGHWORD 1
#define _QUAD_LOWWORD 0

/*
 * Definitions for byte order, according to byte significance from low
 * address to high.
 */

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN   1234    /* LSB first: i386, vax */
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN      4321    /* MSB first: 68000, ibm, net */
#endif

#ifndef PDP_ENDIAN
#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long */
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER      LITTLE_ENDIAN
#endif

#ifndef KERNEL
#include <sys/cdefs.h>
#endif

#include <sys/types.h>

__BEGIN_DECLS
unsigned long   htonl __P((unsigned long));
unsigned short  htons __P((unsigned short));
unsigned long   ntohl __P((unsigned long));
unsigned short  ntohs __P((unsigned short));
__END_DECLS

#define __word_swap_long(x) \
__extension__ ({ register u_long __X = (x); \
   __asm ("rorl $16, %1" \
        : "=r" (__X) \
        : "0" (__X)); \
   __X; })

#if defined(KERNEL) && (defined(I486_CPU) || defined(I586_CPU) || defined(I686_CPU)) && !defined(I386_CPU)

#define __byte_swap_long(x) \
__extension__ ({ register u_long __X = (x); \
   __asm ("bswap %0" \
        : "=r" (__X) \
        : "0" (__X)); \
   __X; })
#else

#define __byte_swap_long(x) \
__extension__ ({ register u_long __X = (x); \
   __asm ("xchgb %h1, %b1\n\trorl $16, %1\n\txchgb %h1, %b1" \
        : "=q" (__X) \
        : "0" (__X)); \
   __X; })
#endif

#define __byte_swap_word(x) \
__extension__ ({ register u_short __X = (x); \
   __asm ("xchgb %h1, %b1" \
        : "=q" (__X) \
        : "0" (__X)); \
   __X; })

/*
 * Macros for network/external number representation conversion.
 */
#ifdef __GNUC__
#define ntohl   __byte_swap_long
#define ntohs   __byte_swap_word
#define htonl   __byte_swap_long
#define htons   __byte_swap_word
#endif

#define NTOHL(x)        ((x) = ntohl((u_long)(x)))
#define NTOHS(x)        ((x) = ntohs((u_short)(x)))
#define HTONL(x)        ((x) = htonl((u_long)(x)))
#define HTONS(x)        ((x) = htons((u_short)(x)))

#endif /* !_MACHINE_ENDIAN_H_ */
