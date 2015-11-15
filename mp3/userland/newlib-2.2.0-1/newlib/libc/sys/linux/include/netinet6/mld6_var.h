/*	$FreeBSD: src/sys/netinet6/mld6_var.h,v 1.2 2000/07/04 16:35:10 itojun Exp $	*/
/*	$KAME: mld6_var.h,v 1.4 2000/03/25 07:23:54 sumikawa Exp $	*/

/*
 * Copyright (C) 1998 WIDE Project.
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

#ifndef _NETINET6_MLD6_VAR_H_
#define _NETINET6_MLD6_VAR_H_

#ifdef _KERNEL

#define MLD6_RANDOM_DELAY(X) (random() % (X) + 1)

/*
 * States for MLD stop-listening processing
 */
#define MLD6_OTHERLISTENER			0
#define MLD6_IREPORTEDLAST			1

void	mld6_init __P((void));
void	mld6_input __P((struct mbuf *, int));
void	mld6_start_listening __P((struct in6_multi *));
void	mld6_stop_listening __P((struct in6_multi *));
void	mld6_fasttimeo __P((void));
#endif /* _KERNEL */

#endif /* _NETINET6_MLD6_VAR_H_ */
