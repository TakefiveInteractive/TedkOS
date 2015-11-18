/*	$FreeBSD: src/sys/netinet6/esp.h,v 1.6 2002/04/19 04:46:22 suz Exp $	*/
/*	$KAME: esp.h,v 1.19 2001/09/04 08:43:19 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * RFC1827/2406 Encapsulated Security Payload.
 */

#ifndef _NETINET6_ESP_H_
#define _NETINET6_ESP_H_

#if defined(_KERNEL) && !defined(_LKM)
#include "opt_inet.h"
#endif

struct esp {
	u_int32_t	esp_spi;	/* ESP */
	/* variable size, 32bit bound */	/* Initialization Vector */
	/* variable size */		/* Payload data */
	/* variable size */		/* padding */
	/* 8bit */			/* pad size */
	/* 8bit */			/* next header */
	/* 8bit */			/* next header */
	/* variable size, 32bit bound */ /* Authentication data (new IPsec) */
};

struct newesp {
	u_int32_t	esp_spi;	/* ESP */
	u_int32_t	esp_seq;	/* Sequence number */
	/* variable size */		/* (IV and) Payload data */
	/* variable size */		/* padding */
	/* 8bit */			/* pad size */
	/* 8bit */			/* next header */
	/* 8bit */			/* next header */
	/* variable size, 32bit bound *//* Authentication data */
};

struct esptail {
	u_int8_t	esp_padlen;	/* pad length */
	u_int8_t	esp_nxt;	/* Next header */
	/* variable size, 32bit bound *//* Authentication data (new IPsec)*/
};

#ifdef _KERNEL
struct secasvar;

struct esp_algorithm {
	size_t padbound;	/* pad boundary, in byte */
	int ivlenval;		/* iv length, in byte */
	int (*mature) __P((struct secasvar *));
	int keymin;	/* in bits */
	int keymax;	/* in bits */
	int (*schedlen) __P((const struct esp_algorithm *));
	const char *name;
	int (*ivlen) __P((const struct esp_algorithm *, struct secasvar *));
	int (*decrypt) __P((struct mbuf *, size_t,
		struct secasvar *, const struct esp_algorithm *, int));
	int (*encrypt) __P((struct mbuf *, size_t, size_t,
		struct secasvar *, const struct esp_algorithm *, int));
	/* not supposed to be called directly */
	int (*schedule) __P((const struct esp_algorithm *, struct secasvar *));
	int (*blockdecrypt) __P((const struct esp_algorithm *,
		struct secasvar *, u_int8_t *, u_int8_t *));
	int (*blockencrypt) __P((const struct esp_algorithm *,
		struct secasvar *, u_int8_t *, u_int8_t *));
};

extern const struct esp_algorithm *esp_algorithm_lookup __P((int));
extern int esp_max_ivlen __P((void));

/* crypt routines */
extern int esp4_output __P((struct mbuf *, struct ipsecrequest *));
extern void esp4_input __P((struct mbuf *, int));
extern size_t esp_hdrsiz __P((struct ipsecrequest *));

extern int esp_schedule __P((const struct esp_algorithm *, struct secasvar *));
extern int esp_auth __P((struct mbuf *, size_t, size_t,
	struct secasvar *, u_char *));
#endif /* _KERNEL */

#endif /* _NETINET6_ESP_H_ */
