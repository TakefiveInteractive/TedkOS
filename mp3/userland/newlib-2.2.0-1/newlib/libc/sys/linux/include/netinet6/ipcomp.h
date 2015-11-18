/*	$FreeBSD: src/sys/netinet6/ipcomp.h,v 1.4 2002/04/19 04:46:23 suz Exp $	*/
/*	$KAME: ipcomp.h,v 1.11 2001/09/04 08:43:19 itojun Exp $	*/

/*
 * Copyright (C) 1999 WIDE Project.
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
 * RFC2393 IP payload compression protocol (IPComp).
 */

#ifndef _NETINET6_IPCOMP_H_
#define _NETINET6_IPCOMP_H_

#if defined(_KERNEL) && !defined(_LKM)
#include "opt_inet.h"
#endif

struct ipcomp {
	u_int8_t comp_nxt;	/* Next Header */
	u_int8_t comp_flags;	/* reserved, must be zero */
	u_int16_t comp_cpi;	/* Compression parameter index */
};

/* well-known algorithm number (in CPI), from RFC2409 */
#define IPCOMP_OUI	1	/* vendor specific */
#define IPCOMP_DEFLATE	2	/* RFC2394 */
#define IPCOMP_LZS	3	/* RFC2395 */
#define IPCOMP_MAX	4

#define IPCOMP_CPI_NEGOTIATE_MIN	256

#ifdef _KERNEL
struct ipcomp_algorithm {
	int (*compress) __P((struct mbuf *, struct mbuf *, size_t *));
	int (*decompress) __P((struct mbuf *, struct mbuf *, size_t *));
	size_t minplen;		/* minimum required length for compression */
};

struct ipsecrequest;
extern const struct ipcomp_algorithm *ipcomp_algorithm_lookup __P((int));
extern void ipcomp4_input __P((struct mbuf *, int));
extern int ipcomp4_output __P((struct mbuf *, struct ipsecrequest *));
#endif /* KERNEL */

#endif /* _NETINET6_IPCOMP_H_ */
