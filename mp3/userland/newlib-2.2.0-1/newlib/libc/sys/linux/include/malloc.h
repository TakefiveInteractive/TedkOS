/* Prototypes and definition for malloc implementation.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _MALLOC_H
#define _MALLOC_H 1

#include <features.h>

/*
  `ptmalloc', a malloc implementation for multiple threads without
  lock contention, by Wolfram Gloger <wmglo@dent.med.uni-muenchen.de>.
  See the files `ptmalloc.c' or `COPYRIGHT' for copying conditions.

  VERSION 2.6.4-pt Wed Dec  4 00:35:54 MET 1996

  This work is mainly derived from malloc-2.6.4 by Doug Lea
  <dl@cs.oswego.edu>, which is available from:

                 ftp://g.oswego.edu/pub/misc/malloc.c

  This trimmed-down header file only provides function prototypes and
  the exported data structures.  For more detailed function
  descriptions and compile-time options, see the source file
  `ptmalloc.c'.
*/

#if defined(__STDC__) || defined (__cplusplus)
# include <stddef.h>
# define __malloc_ptr_t  void *
#else
# undef  size_t
# define size_t          unsigned int
# undef  ptrdiff_t
# define ptrdiff_t       int
# define __malloc_ptr_t  char *
#endif

#ifdef _LIBC
/* Used by GNU libc internals. */
# define __malloc_size_t size_t
# define __malloc_ptrdiff_t ptrdiff_t
#elif !defined __attribute_malloc__
# define __attribute_malloc__
#endif

#ifdef __GNUC__

/* GCC can always grok prototypes.  For C++ programs we add throw()
   to help it optimize the function calls.  But this works only with
   gcc 2.8.x and egcs.  */
# if defined __cplusplus && (__GNUC__ >= 3 || __GNUC_MINOR__ >= 8)
#  define __THROW	throw ()
# else
#  define __THROW
# endif
# define __MALLOC_P(args)	args __THROW
/* This macro will be used for functions which might take C++ callback
   functions.  */
# define __MALLOC_PMT(args)	args

#else	/* Not GCC.  */

# define __THROW

# if (defined __STDC__ && __STDC__) || defined __cplusplus

#  define __MALLOC_P(args)	args
#  define __MALLOC_PMT(args)	args

# else	/* Not ANSI C or C++.  */

#  define __MALLOC_P(args)	()	/* No prototypes.  */
#  define __MALLOC_PMT(args)	()

# endif	/* ANSI C or C++.  */

#endif	/* GCC.  */

#ifndef NULL
# ifdef __cplusplus
#  define NULL	0
# else
#  define NULL	((__malloc_ptr_t) 0)
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct _reent;

/* Nonzero if the malloc is already initialized.  */
#ifdef _LIBC
/* In the GNU libc we rename the global variable
   `__malloc_initialized' to `__libc_malloc_initialized'.  */
# define __malloc_initialized __libc_malloc_initialized
#endif
extern int __malloc_initialized;

/* Initialize global configuration.  Not needed with GNU libc. */
#ifndef __GLIBC__
extern void ptmalloc_init __MALLOC_P ((void));
#endif

/* Allocate SIZE bytes of memory.  */
extern __malloc_ptr_t malloc __MALLOC_P ((size_t __size)) __attribute_malloc__;
extern __malloc_ptr_t _malloc_r __MALLOC_P ((struct _reent * __r, size_t __size)) __attribute_malloc__;

/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern __malloc_ptr_t calloc __MALLOC_P ((size_t __nmemb, size_t __size))
       __attribute_malloc__;
extern __malloc_ptr_t _calloc_r __MALLOC_P ((struct _reent * __r,
                                             size_t __nmemb, size_t __size))
       __attribute_malloc__;

/* Re-allocate the previously allocated block in __ptr, making the new
   block SIZE bytes long.  */
extern __malloc_ptr_t realloc __MALLOC_P ((__malloc_ptr_t __ptr,
					   size_t __size))
       __attribute_malloc__;
extern __malloc_ptr_t _realloc_r __MALLOC_P ((struct _reent * __r,
			         	      __malloc_ptr_t __ptr,
				              size_t __size))
       __attribute_malloc__;

/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free __MALLOC_P ((__malloc_ptr_t __ptr));
extern void _free_r __MALLOC_P ((struct _reent * __r, __malloc_ptr_t __ptr));

/* Free a block allocated by `calloc'. */
extern void cfree __MALLOC_P ((__malloc_ptr_t __ptr));

/* Allocate SIZE bytes allocated to ALIGNMENT bytes.  */
extern __malloc_ptr_t memalign __MALLOC_P ((size_t __alignment, size_t __size));
extern __malloc_ptr_t _memalign_r __MALLOC_P ((struct _reent *__r,
                                               size_t __alignment, 
                                               size_t __size));

/* Allocate SIZE bytes on a page boundary.  */
extern __malloc_ptr_t valloc __MALLOC_P ((size_t __size)) __attribute_malloc__;
extern __malloc_ptr_t _valloc_r __MALLOC_P ((struct _reent *__r,
                                             size_t __size)) 
       __attribute_malloc__;

/* Equivalent to valloc(minimum-page-that-holds(n)), that is, round up
   __size to nearest pagesize. */
extern __malloc_ptr_t  pvalloc __MALLOC_P ((size_t __size))
       __attribute_malloc__;
extern __malloc_ptr_t  _pvalloc_r __MALLOC_P ((struct _reent *__r,
                                               size_t __size))
       __attribute_malloc__;

/* Underlying allocation function; successive calls should return
   contiguous pieces of memory.  */
extern __malloc_ptr_t (*__morecore) __MALLOC_PMT ((ptrdiff_t __size));

/* Default value of `__morecore'.  */
extern __malloc_ptr_t __default_morecore __MALLOC_P ((ptrdiff_t __size))
       __attribute_malloc__;

/* SVID2/XPG mallinfo structure */
struct mallinfo {
  int arena;    /* total space allocated from system */
  int ordblks;  /* number of non-inuse chunks */
  int smblks;   /* unused -- always zero */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* total space in mmapped regions */
  int usmblks;  /* unused -- always zero */
  int fsmblks;  /* unused -- always zero */
  int uordblks; /* total allocated space */
  int fordblks; /* total non-inuse space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

/* Returns a copy of the updated current mallinfo. */
extern struct mallinfo mallinfo __MALLOC_P ((void));
extern struct mallinfo _mallinfo_r __MALLOC_P ((struct _reent *__r));

/* SVID2/XPG mallopt options */
#ifndef M_MXFAST
# define M_MXFAST  1	/* UNUSED in this malloc */
#endif
#ifndef M_NLBLKS
# define M_NLBLKS  2	/* UNUSED in this malloc */
#endif
#ifndef M_GRAIN
# define M_GRAIN   3	/* UNUSED in this malloc */
#endif
#ifndef M_KEEP
# define M_KEEP    4	/* UNUSED in this malloc */
#endif

/* mallopt options that actually do something */
#define M_TRIM_THRESHOLD    -1
#define M_TOP_PAD           -2
#define M_MMAP_THRESHOLD    -3
#define M_MMAP_MAX          -4
#define M_CHECK_ACTION      -5

/* General SVID/XPG interface to tunable parameters. */
extern int mallopt __MALLOC_P ((int __param, int __val));
extern int _mallopt_r __MALLOC_P ((struct _reent *__r, int __param, int __val));

/* Release all but __pad bytes of freed top-most memory back to the
   system. Return 1 if successful, else 0. */
extern int malloc_trim __MALLOC_P ((size_t __pad));

/* Report the number of usable allocated bytes associated with allocated
   chunk __ptr. */
extern size_t malloc_usable_size __MALLOC_P ((__malloc_ptr_t __ptr));

/* Prints brief summary statistics on stderr. */
extern void malloc_stats __MALLOC_P ((void));
extern void _malloc_stats_r __MALLOC_P ((struct _reent *__r));

/* Record the state of all malloc variables in an opaque data structure. */
extern __malloc_ptr_t malloc_get_state __MALLOC_P ((void));

/* Restore the state of all malloc variables from data obtained with
   malloc_get_state(). */
extern int malloc_set_state __MALLOC_P ((__malloc_ptr_t __ptr));

#if defined __GLIBC__ || defined MALLOC_HOOKS
/* Called once when malloc is initialized; redefining this variable in
   the application provides the preferred way to set up the hook
   pointers. */
extern void (*__malloc_initialize_hook) __MALLOC_PMT ((void));
/* Hooks for debugging and user-defined versions. */
extern void (*__free_hook) __MALLOC_PMT ((__malloc_ptr_t __ptr,
					__const __malloc_ptr_t));
extern __malloc_ptr_t (*__malloc_hook) __MALLOC_PMT ((size_t __size,
						    __const __malloc_ptr_t));
extern __malloc_ptr_t (*__realloc_hook) __MALLOC_PMT ((__malloc_ptr_t __ptr,
						     size_t __size,
						     __const __malloc_ptr_t));
extern __malloc_ptr_t (*__memalign_hook) __MALLOC_PMT ((size_t __alignment,
						      size_t __size,
						      __const __malloc_ptr_t));
extern void (*__after_morecore_hook) __MALLOC_PMT ((void));

/* Activate a standard set of debugging hooks. */
extern void __malloc_check_init __MALLOC_P ((void));
#endif

#ifdef __cplusplus
}; /* end of extern "C" */
#endif

#endif /* malloc.h */
