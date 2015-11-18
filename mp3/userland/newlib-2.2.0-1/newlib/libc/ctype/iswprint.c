/* Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

     Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse
     or promote products derived from this software without specific
     prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
FUNCTION
	<<iswprint>>---printable wide character test

INDEX
	iswprint

ANSI_SYNOPSIS
	#include <wctype.h>
	int iswprint(wint_t <[c]>);

TRAD_SYNOPSIS
	#include <wctype.h>
	int iswprint(<[c]>)
	wint_t <[c]>;

DESCRIPTION
<<iswprint>> is a function which classifies wide-character values that
are printable.

RETURNS
<<iswprint>> returns non-zero if <[c]> is a printable wide character.

PORTABILITY
<<iswprint>> is C99.

No supporting OS subroutines are required.
*/
#include <_ansi.h>
#include <newlib.h>
#include <wctype.h>
#include <string.h>
#include <ctype.h>
#include "local.h"

#ifdef _MB_CAPABLE
#include "utf8print.h"
#endif /* _MB_CAPABLE */

int
_DEFUN(iswprint,(c), wint_t c)
{
#ifdef _MB_CAPABLE
  unsigned const char *table;
  unsigned char *ptr;
  unsigned char ctmp;
  int size;
  wint_t x;
  
  c = _jp2uc (c);

  /* Based on and tested against Unicode 5.2
     See utf8print.h for a description how to fetch the data. */
  x = (c >> 8);
  /* for some large sections, all characters are printuation so handle them here */
  if ((x >= 0x33 && x <= 0x4c) ||
      (x >= 0x4e && x <= 0x9e) ||
      (x >= 0xa0 && x <= 0xa3) ||
      (x >= 0xac && x <= 0xd6) ||
      (x >= 0xe0 && x <= 0xf9) ||
      (x >= 0x120 && x <= 0x122) ||
      (x >= 0x130 && x <= 0x133) ||
      (x >= 0x200 && x <= 0x2a5) ||
      (x >= 0x2a7 && x <= 0x2b6) ||
      (x >= 0xf00 && x <= 0xffe) ||
      (x >= 0x1000 && x <= 0x10fe))
    return 1;
  
  switch (x)
    {
    case 0x01:
    case 0x02:
    case 0x04:
    case 0x11:
    case 0x14:
    case 0x15:
    case 0x1e:
    case 0x22:
    case 0x25:
    case 0x28:
    case 0x29:
    case 0x2a:
    case 0xa5:
    case 0xfc:
    case 0x2f8:
    case 0x2f9:
      return 1;
    case 0x00:
      table = u0;
      size = sizeof(u0);
      break;
    case 0x03:
      table = u3;
      size = sizeof(u3);
      break;
    case 0x05:
      table = u5;
      size = sizeof(u5);
      break;
    case 0x06:
      table = u6;
      size = sizeof(u6);
      break;
    case 0x07:
      table = u7;
      size = sizeof(u7);
      break;
    case 0x08:
      table = u8;
      size = sizeof(u8);
      break;
    case 0x09:
      table = u9;
      size = sizeof(u9);
      break;
    case 0x0a:
      table = ua;
      size = sizeof(ua);
      break;
    case 0x0b:
      table = ub;
      size = sizeof(ub);
      break;
    case 0x0c:
      table = uc;
      size = sizeof(uc);
      break;
    case 0x0d:
      table = ud;
      size = sizeof(ud);
      break;
    case 0x0e:
      table = ue;
      size = sizeof(ue);
      break;
    case 0x0f:
      table = uf;
      size = sizeof(uf);
      break;
    case 0x10:
      table = u10;
      size = sizeof(u10);
      break;
    case 0x12:
      table = u12;
      size = sizeof(u12);
      break;
    case 0x13:
      table = u13;
      size = sizeof(u13);
      break;
    case 0x16:
      table = u16;
      size = sizeof(u16);
      break;
    case 0x17:
      table = u17;
      size = sizeof(u17);
      break;
    case 0x18:
      table = u18;
      size = sizeof(u18);
      break;
    case 0x19:
      table = u19;
      size = sizeof(u19);
      break;
    case 0x1a:
      table = u1a;
      size = sizeof(u1a);
      break;
    case 0x1b:
      table = u1b;
      size = sizeof(u1b);
      break;
    case 0x1c:
      table = u1c;
      size = sizeof(u1c);
      break;
    case 0x1d:
      table = u1d;
      size = sizeof(u1d);
      break;
    case 0x1f:
      table = u1f;
      size = sizeof(u1f);
      break;
    case 0x20:
      table = u20;
      size = sizeof(u20);
      break;
    case 0x21:
      table = u21;
      size = sizeof(u21);
      break;
    case 0x23:
      table = u23;
      size = sizeof(u23);
      break;
    case 0x24:
      table = u24;
      size = sizeof(u24);
      break;
    case 0x26:
      table = u26;
      size = sizeof(u26);
      break;
    case 0x27:
      table = u27;
      size = sizeof(u27);
      break;
    case 0x2b:
      table = u2b;
      size = sizeof(u2b);
      break;
    case 0x2c:
      table = u2c;
      size = sizeof(u2c);
      break;
    case 0x2d:
      table = u2d;
      size = sizeof(u2d);
      break;
    case 0x2e:
      table = u2e;
      size = sizeof(u2e);
      break;
    case 0x2f:
      table = u2f;
      size = sizeof(u2f);
      break;
    case 0x30:
      table = u30;
      size = sizeof(u30);
      break;
    case 0x31:
      table = u31;
      size = sizeof(u31);
      break;
    case 0x32:
      table = u32;
      size = sizeof(u32);
      break;
    case 0x4d:
      table = u4d;
      size = sizeof(u4d);
      break;
    case 0x9f:
      table = u9f;
      size = sizeof(u9f);
      break;
    case 0xa4:
      table = ua4;
      size = sizeof(ua4);
      break;
    case 0xa6:
      table = ua6;
      size = sizeof(ua6);
      break;
    case 0xa7:
      table = ua7;
      size = sizeof(ua7);
      break;
    case 0xa8:
      table = ua8;
      size = sizeof(ua8);
      break;
    case 0xa9:
      table = ua9;
      size = sizeof(ua9);
      break;
    case 0xaa:
      table = uaa;
      size = sizeof(uaa);
      break;
    case 0xab:
      table = uab;
      size = sizeof(uab);
      break;
    case 0xd7:
      table = ud7;
      size = sizeof(ud7);
      break;
    case 0xfa:
      table = ufa;
      size = sizeof(ufa);
      break;
    case 0xfb:
      table = ufb;
      size = sizeof(ufb);
      break;
    case 0xfd:
      table = ufd;
      size = sizeof(ufd);
      break;
    case 0xfe:
      table = ufe;
      size = sizeof(ufe);
      break;
    case 0xff:
      table = uff;
      size = sizeof(uff);
      break;
    case 0x100:
      table = u100;
      size = sizeof(u100);
      break;
    case 0x101:
      table = u101;
      size = sizeof(u101);
      break;
    case 0x102:
      table = u102;
      size = sizeof(u102);
      break;
    case 0x103:
      table = u103;
      size = sizeof(u103);
      break;
    case 0x104:
      table = u104;
      size = sizeof(u104);
      break;
    case 0x108:
      table = u108;
      size = sizeof(u108);
      break;
    case 0x109:
      table = u109;
      size = sizeof(u109);
      break;
    case 0x10a:
      table = u10a;
      size = sizeof(u10a);
      break;
    case 0x10b:
      table = u10b;
      size = sizeof(u10b);
      break;
    case 0x10c:
      table = u10c;
      size = sizeof(u10c);
      break;
    case 0x10e:
      table = u10e;
      size = sizeof(u10e);
      break;
    case 0x110:
      table = u110;
      size = sizeof(u110);
      break;
    case 0x123:
      table = u123;
      size = sizeof(u123);
      break;
    case 0x124:
      table = u124;
      size = sizeof(u124);
      break;
    case 0x134:
      table = u134;
      size = sizeof(u134);
      break;
    case 0x1d0:
      table = u1d0;
      size = sizeof(u1d0);
      break;
    case 0x1d1:
      table = u1d1;
      size = sizeof(u1d1);
      break;
    case 0x1d2:
      table = u1d2;
      size = sizeof(u1d2);
      break;
    case 0x1d3:
      table = u1d3;
      size = sizeof(u1d3);
      break;
    case 0x1d4:
      table = u1d4;
      size = sizeof(u1d4);
      break;
    case 0x1d5:
      table = u1d5;
      size = sizeof(u1d5);
      break;
    case 0x1d6:
      table = u1d6;
      size = sizeof(u1d6);
      break;
    case 0x1d7:
      table = u1d7;
      size = sizeof(u1d7);
      break;
    case 0x1f0:
      table = u1f0;
      size = sizeof(u1f0);
      break;
    case 0x1f1:
      table = u1f1;
      size = sizeof(u1f1);
      break;
    case 0x1f2:
      table = u1f2;
      size = sizeof(u1f2);
      break;
    case 0x2a6:
      table = u2a6;
      size = sizeof(u2a6);
      break;
    case 0x2b7:
      table = u2b7;
      size = sizeof(u2b7);
      break;
    case 0x2fa:
      table = u2fa;
      size = sizeof(u2fa);
      break;
    case 0xe00:
      table = ue00;
      size = sizeof(ue00);
      break;
    case 0xe01:
      table = ue01;
      size = sizeof(ue01);
      break;
    case 0xfff:
      table = ufff;
      size = sizeof(ufff);
      break;
    case 0x10ff:
      table = u10ff;
      size = sizeof(u10ff);
      break;
    default:
      return 0;
    }
  /* we have narrowed down to a section of 256 characters to check */
  /* now check if c matches the printuation wide-chars within that section */
  ptr = (unsigned char *)table;
  ctmp = (unsigned char)c;
  while (ptr < table + size)
    {
      if (ctmp == *ptr)
	return 1;
      if (ctmp < *ptr)
	return 0;
      /* otherwise c > *ptr */
      /* look for 0x0 as next element which indicates a range */
      ++ptr;
      if (*ptr == 0x0)
	{
	  /* we have a range..see if c falls within range */
	  ++ptr;
	  if (ctmp <= *ptr)
	    return 1;
	  ++ptr;
	}
    }
  /* not in table */
  return 0;
#else
  return (c < (wint_t)0x100 ? isprint (c) : 0);
#endif /* _MB_CAPABLE */
}

