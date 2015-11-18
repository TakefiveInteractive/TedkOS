/*
 * Code created by modifying scanf.c which has following copyright.
 *
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <string.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "local.h"

/*
 * vsscanf
 */

#ifndef _REENT_ONLY

int
_DEFUN(vsscanf, (str, fmt, ap), 
       _CONST char *__restrict str _AND 
       _CONST char *__restrict fmt _AND 
       va_list ap)
{
  return _vsscanf_r (_REENT, str, fmt, ap);
}

#ifdef _NANO_FORMATTED_IO
int
_EXFUN(vsiscanf, (const char *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("vsscanf"))));
#endif

#endif /* !_REENT_ONLY */

int
_DEFUN(_vsscanf_r, (ptr, str, fmt, ap),
       struct _reent *ptr _AND 
       _CONST char *__restrict str   _AND 
       _CONST char *__restrict fmt   _AND 
       va_list ap)
{
  FILE f;

  f._flags = __SRD | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._r = strlen (str);
  f._read = __seofread;
  f._ub._base = NULL;
  f._lb._base = NULL;
  f._file = -1;  /* No file. */
  return __ssvfscanf_r (ptr, &f, fmt, ap);
}

#ifdef _NANO_FORMATTED_IO
int
_EXFUN(_vsiscanf_r, (struct _reent *, const char *, const char *, __VALIST)
       _ATTRIBUTE ((__alias__("_vsscanf_r"))));
#endif
