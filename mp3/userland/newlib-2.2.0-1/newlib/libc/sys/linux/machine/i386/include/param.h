/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 *	from: @(#)param.h	5.8 (Berkeley) 6/28/91
 * $FreeBSD: src/sys/i386/include/param.h,v 1.62 2001/09/12 08:37:33 julian Exp $
 */

/*
 * Machine dependent constants for Intel 386.
 */

#ifndef _MACHINE_PARAM_H_
#define	_MACHINE_PARAM_H_

#ifndef MACHINE
#define MACHINE		"i386"
#endif
#ifndef MACHINE_ARCH
#define	MACHINE_ARCH	"i386"
#endif
#define MID_MACHINE	MID_I386

/* Endian-ness */
#ifndef BIG_ENDIAN
#define BIG_ENDIAN	4321
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN	1234
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER	LITTLE_ENDIAN
#endif

/*
 * OBJFORMAT_NAMES is a comma-separated list of the object formats
 * that are supported on the architecture.
 */
#define OBJFORMAT_NAMES		"elf", "aout"
#define OBJFORMAT_DEFAULT	"elf"

#define MAXHOSTNAMELEN 64

#ifdef SMP
#define MAXCPU		16
#else
#define MAXCPU		1
#endif /* SMP */

#define ALIGNBYTES	_ALIGNBYTES
#define ALIGN(p)	_ALIGN(p)

#define NPTEPG		(PAGE_SIZE/(sizeof (pt_entry_t)))

#define NPDEPG		(PAGE_SIZE/(sizeof (pd_entry_t)))
#define PDRSHIFT	22		/* LOG2(NBPDR) */
#define NBPDR		(1<<PDRSHIFT)	/* bytes/page dir */
#define PDRMASK		(NBPDR-1)

#define DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define DEV_BSIZE	(1<<DEV_BSHIFT)

#ifndef BLKDEV_IOSIZE
#define BLKDEV_IOSIZE	PAGE_SIZE	/* default block device I/O size */
#endif
#define DFLTPHYS	(64 * 1024)	/* default max raw I/O transfer size */
#define MAXPHYS		(128 * 1024)	/* max raw I/O transfer size */
#define MAXDUMPPGS	(DFLTPHYS/PAGE_SIZE)

#define IOPAGES	2		/* pages of i/o permission bitmap */

#ifndef KSTACK_PAGES
#define KSTACK_PAGES 2		/* Includes pcb! */
#endif
#define UAREA_PAGES 1		/* holds struct user WITHOUT PCB (see def.) */

#define KSTACK_GUARD		/* compile in the kstack guard page */

/*
 * Ceiling on amount of swblock kva space.
 */
#ifndef VM_SWZONE_SIZE_MAX
#define VM_SWZONE_SIZE_MAX	(70 * 1024 * 1024)
#endif

/*
 * Ceiling on size of buffer cache (really only effects write queueing,
 * the VM page cache is not effected).
 */
#ifndef VM_BCACHE_SIZE_MAX
#define VM_BCACHE_SIZE_MAX	(200 * 1024 * 1024)
#endif


/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than PAGE_SIZE.
 */
#ifndef	MSIZE
#define MSIZE		256		/* size of an mbuf */
#endif	/* MSIZE */

#ifndef	MCLSHIFT
#define MCLSHIFT	11		/* convert bytes to mbuf clusters */
#endif	/* MCLSHIFT */
#define MCLBYTES	(1 << MCLSHIFT)	/* size of an mbuf cluster */

/*
 * Some macros for units conversion
 */

/* clicks to bytes */
#define ctob(x)	((x)<<PAGE_SHIFT)

/* bytes to clicks */
#define btoc(x)	(((unsigned)(x)+PAGE_MASK)>>PAGE_SHIFT)

/*
 * btodb() is messy and perhaps slow because `bytes' may be an off_t.  We
 * want to shift an unsigned type to avoid sign extension and we don't
 * want to widen `bytes' unnecessarily.  Assume that the result fits in
 * a daddr_t.
 */
#define btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	(sizeof (bytes) > sizeof(long) \
	 ? (daddr_t)((unsigned long long)(bytes) >> DEV_BSHIFT) \
	 : (daddr_t)((unsigned long)(bytes) >> DEV_BSHIFT))

#define dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((off_t)(db) << DEV_BSHIFT)

/*
 * Mach derived conversion macros
 */
#define trunc_page(x)		((x) & ~PAGE_MASK)
#define round_page(x)		(((x) + PAGE_MASK) & ~PAGE_MASK)
#define trunc_4mpage(x)		((unsigned)(x) & ~PDRMASK)
#define round_4mpage(x)		((((unsigned)(x)) + PDRMASK) & ~PDRMASK)

#define atop(x)			((unsigned)(x) >> PAGE_SHIFT)
#define ptoa(x)			((unsigned)(x) << PAGE_SHIFT)

#define i386_btop(x)		((unsigned)(x) >> PAGE_SHIFT)
#define i386_ptob(x)		((unsigned)(x) << PAGE_SHIFT)

#define	pgtok(x)		((x) * (PAGE_SIZE / 1024))

#endif /* !_MACHINE_PARAM_H_ */
