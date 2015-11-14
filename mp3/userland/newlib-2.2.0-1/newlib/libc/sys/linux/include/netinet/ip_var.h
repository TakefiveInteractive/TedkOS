/*
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *	@(#)ip_var.h	8.2 (Berkeley) 1/9/95
 * $FreeBSD: src/sys/netinet/ip_var.h,v 1.60 2002/03/19 21:25:46 alfred Exp $
 */

#ifndef _NETINET_IP_VAR_H_
#define	_NETINET_IP_VAR_H_

#include <sys/queue.h>

/*
 * Overlay for ip header used by other protocols (tcp, udp).
 */
struct ipovly {
	u_char	ih_x1[9];		/* (unused) */
	u_char	ih_pr;			/* protocol */
	u_short	ih_len;			/* protocol length */
	struct	in_addr ih_src;		/* source internet address */
	struct	in_addr ih_dst;		/* destination internet address */
};

/*
 * Ip reassembly queue structure.  Each fragment
 * being reassembled is attached to one of these structures.
 * They are timed out after ipq_ttl drops to 0, and may also
 * be reclaimed if memory becomes tight.
 */
struct ipq {
	TAILQ_ENTRY(ipq) ipq_list;	/* to other reass headers */
	u_char	ipq_ttl;		/* time for reass q to live */
	u_char	ipq_p;			/* protocol of this fragment */
	u_short	ipq_id;			/* sequence id for reassembly */
	struct mbuf *ipq_frags;		/* to ip headers of fragments */
	struct	in_addr ipq_src,ipq_dst;
#ifdef IPDIVERT
	u_int32_t ipq_div_info;		/* ipfw divert port & flags */
	u_int16_t ipq_div_cookie;	/* ipfw divert cookie */
#endif
};

/*
 * Structure stored in mbuf in inpcb.ip_options
 * and passed to ip_output when ip options are in use.
 * The actual length of the options (including ipopt_dst)
 * is in m_len.
 */
#define MAX_IPOPTLEN	40

struct ipoption {
	struct	in_addr ipopt_dst;	/* first-hop dst if source routed */
	char	ipopt_list[MAX_IPOPTLEN];	/* options proper */
};

/*
 * Structure attached to inpcb.ip_moptions and
 * passed to ip_output when IP multicast options are in use.
 */
struct ip_moptions {
	struct	ifnet *imo_multicast_ifp; /* ifp for outgoing multicasts */
	struct in_addr imo_multicast_addr; /* ifindex/addr on MULTICAST_IF */
	u_char	imo_multicast_ttl;	/* TTL for outgoing multicasts */
	u_char	imo_multicast_loop;	/* 1 => hear sends if a member */
	u_short	imo_num_memberships;	/* no. memberships this socket */
	struct	in_multi *imo_membership[IP_MAX_MEMBERSHIPS];
	u_long	imo_multicast_vif;	/* vif num outgoing multicasts */
};

struct	ipstat {
	u_long	ips_total;		/* total packets received */
	u_long	ips_badsum;		/* checksum bad */
	u_long	ips_tooshort;		/* packet too short */
	u_long	ips_toosmall;		/* not enough data */
	u_long	ips_badhlen;		/* ip header length < data size */
	u_long	ips_badlen;		/* ip length < ip header length */
	u_long	ips_fragments;		/* fragments received */
	u_long	ips_fragdropped;	/* frags dropped (dups, out of space) */
	u_long	ips_fragtimeout;	/* fragments timed out */
	u_long	ips_forward;		/* packets forwarded */
	u_long	ips_fastforward;	/* packets fast forwarded */
	u_long	ips_cantforward;	/* packets rcvd for unreachable dest */
	u_long	ips_redirectsent;	/* packets forwarded on same net */
	u_long	ips_noproto;		/* unknown or unsupported protocol */
	u_long	ips_delivered;		/* datagrams delivered to upper level*/
	u_long	ips_localout;		/* total ip packets generated here */
	u_long	ips_odropped;		/* lost packets due to nobufs, etc. */
	u_long	ips_reassembled;	/* total packets reassembled ok */
	u_long	ips_fragmented;		/* datagrams successfully fragmented */
	u_long	ips_ofragments;		/* output fragments created */
	u_long	ips_cantfrag;		/* don't fragment flag was set, etc. */
	u_long	ips_badoptions;		/* error in option processing */
	u_long	ips_noroute;		/* packets discarded due to no route */
	u_long	ips_badvers;		/* ip version != 4 */
	u_long	ips_rawout;		/* total raw ip packets generated */
	u_long	ips_toolong;		/* ip length > max ip packet size */
	u_long	ips_notmember;		/* multicasts for unregistered grps */
	u_long	ips_nogif;		/* no match gif found */
	u_long	ips_badaddr;		/* invalid address on header */
};

#ifdef _KERNEL

/* flags passed to ip_output as last parameter */
#define	IP_FORWARDING		0x1		/* most of ip header exists */
#define	IP_RAWOUTPUT		0x2		/* raw ip header exists */
#define	IP_ROUTETOIF		SO_DONTROUTE	/* bypass routing tables */
#define	IP_ALLOWBROADCAST	SO_BROADCAST	/* can send broadcast packets */

struct ip;
struct inpcb;
struct route;
struct sockopt;

extern struct	ipstat	ipstat;
#ifndef RANDOM_IP_ID
extern u_short	ip_id;				/* ip packet ctr, for ids */
#endif
extern int	ip_defttl;			/* default IP ttl */
extern int	ipforwarding;			/* ip forwarding */
extern struct route ipforward_rt;		/* ip forwarding cached route */
extern u_char	ip_protox[];
extern struct socket *ip_rsvpd;	/* reservation protocol daemon */
extern struct socket *ip_mrouter; /* multicast routing daemon */
extern int	(*legal_vif_num)(int);
extern u_long	(*ip_mcast_src)(int);
extern int rsvp_on;
extern struct	pr_usrreqs rip_usrreqs;

int	 ip_ctloutput(struct socket *, struct sockopt *sopt);
void	 ip_drain(void);
void	 ip_freemoptions(struct ip_moptions *);
void	 ip_init(void);
extern int	 (*ip_mforward)(struct ip *, struct ifnet *, struct mbuf *,
			  struct ip_moptions *);
int	 ip_output(struct mbuf *,
	    struct mbuf *, struct route *, int, struct ip_moptions *);
struct in_ifaddr *
	 ip_rtaddr(struct in_addr, struct route *);
void	 ip_savecontrol(struct inpcb *, struct mbuf **, struct ip *,
		struct mbuf *);
void	 ip_slowtimo(void);
struct mbuf *
	 ip_srcroute(void);
void	 ip_stripoptions(struct mbuf *, struct mbuf *);
#ifdef RANDOM_IP_ID
u_int16_t	
	 ip_randomid(void);
#endif
int	 rip_ctloutput(struct socket *, struct sockopt *);
void	 rip_ctlinput(int, struct sockaddr *, void *);
void	 rip_init(void);
void	 rip_input(struct mbuf *, int);
int	 rip_output(struct mbuf *, struct socket *, u_long);
void	ipip_input(struct mbuf *, int);
void	rsvp_input(struct mbuf *, int);
int	ip_rsvp_init(struct socket *);
int	ip_rsvp_done(void);
int	ip_rsvp_vif_init(struct socket *, struct sockopt *);
int	ip_rsvp_vif_done(struct socket *, struct sockopt *);
void	ip_rsvp_force_done(struct socket *);

#ifdef IPDIVERT
void	div_init(void);
void	div_input(struct mbuf *, int);
void	divert_packet(struct mbuf *, int, int);
extern struct pr_usrreqs div_usrreqs;
extern u_int16_t ip_divert_cookie;
#endif

extern struct sockaddr_in *ip_fw_fwd_addr;

void	in_delayed_cksum(struct mbuf *m);

#endif /* _KERNEL */

#endif /* !_NETINET_IP_VAR_H_ */
