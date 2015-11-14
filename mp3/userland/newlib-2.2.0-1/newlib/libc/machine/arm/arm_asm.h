/*
 * Copyright (c) 2009 ARM Ltd
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
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ARM_ASM__H
#define ARM_ASM__H

#include "acle-compat.h"

#if __ARM_ARCH >= 7 && defined (__ARM_ARCH_ISA_ARM)
# define _ISA_ARM_7
#endif

#if __ARM_ARCH >= 6 && defined (__ARM_ARCH_ISA_ARM)
# define _ISA_ARM_6
#endif

#if __ARM_ARCH >= 5
# define _ISA_ARM_5
#endif

#if __ARM_ARCH >= 4 && __ARM_ARCH_ISA_THUMB >= 1
# define _ISA_ARM_4T
#endif

#if __ARM_ARCH >= 4 && __ARM_ARCH_ISA_THUMB == 0
# define _ISA_ARM_4
#endif


#if __ARM_ARCH_ISA_THUMB >= 2
# define _ISA_THUMB_2
#endif

#if __ARM_ARCH_ISA_THUMB >= 1
# define _ISA_THUMB_1
#endif


/* Now some macros for common instruction sequences.  */
#ifdef __ASSEMBLER__
.macro  RETURN     cond=
#if defined (_ISA_ARM_4T) || defined (_ISA_THUMB_1)
	bx\cond	lr
#else
	mov\cond pc, lr
#endif
.endm

.macro optpld	base, offset=#0
#if defined (_ISA_ARM_7)
	pld	[\base, \offset]
#endif
.endm

#else
asm(".macro  RETURN	cond=\n\t"
#if defined (_ISA_ARM_4T) || defined (_ISA_THUMB_1)
    "bx\\cond	lr\n\t"
#else
    "mov\\cond	pc, lr\n\t"
#endif
    ".endm"
    );

asm(".macro optpld	base, offset=#0\n\t"
#if defined (_ISA_ARM_7)
    "pld	[\\base, \\offset]\n\t"
#endif
    ".endm"
    );
#endif

#endif /* ARM_ASM__H */
