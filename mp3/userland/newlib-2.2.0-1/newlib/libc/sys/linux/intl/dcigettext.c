/* Implementation of the internal dcigettext function.
   Copyright (C) 1995-1999, 2000, 2001 Free Software Foundation, Inc.
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

#include <newlib.h>

#ifdef _MB_CAPABLE

/* Tell glibc's <string.h> to provide a prototype for mempcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>

#if defined __GNUC__ && !defined C_ALLOCA
# define alloca __builtin_alloca
# define HAVE_ALLOCA 1
#else
# if (defined HAVE_ALLOCA_H || defined _LIBC) && !defined C_ALLOCA
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca
char *alloca ();
#   endif
#  endif
# endif
#endif

#include <errno.h>
#ifndef errno
extern int errno;
#endif
#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#if defined STDC_HEADERS || defined _LIBC
# include <stddef.h>
# include <stdlib.h>
#else
char *getenv ();
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# else
void free ();
# endif
#endif

#if defined HAVE_STRING_H || defined _LIBC
# include <string.h>
#else
# include <strings.h>
#endif
#if !HAVE_STRCHR && !defined _LIBC
# ifndef strchr
#  define strchr index
# endif
#endif

#if defined HAVE_UNISTD_H || defined _LIBC
# include <unistd.h>
#endif

#if defined HAVE_LOCALE_H || defined _LIBC
# include <locale.h>
#endif

#if defined HAVE_SYS_PARAM_H || defined _LIBC
# include <sys/param.h>
#endif

#include "gettextP.h"
#ifdef _LIBC
# include <libintl.h>
#else
# include "libgnuintl.h"
#endif
#include "hash-string.h"

/* Thread safetyness.  */
#ifdef _LIBC
# include <bits/libc-lock.h>
#else
/* Provide dummy implementation if this is outside glibc.  */
# define __libc_lock_define_initialized(CLASS, NAME)
# define __libc_lock_lock(NAME)
# define __libc_lock_unlock(NAME)
# define __libc_rwlock_define_initialized(CLASS, NAME)
# define __libc_rwlock_rdlock(NAME)
# define __libc_rwlock_unlock(NAME)
#endif

/* Alignment of types.  */
#if defined __GNUC__ && __GNUC__ >= 2
# define alignof(TYPE) __alignof__ (TYPE)
#else
# define alignof(TYPE) \
    ((int) &((struct { char dummy1; TYPE dummy2; } *) 0)->dummy2)
#endif

/* The internal variables in the standalone libintl.a must have different
   names than the internal variables in GNU libc, otherwise programs
   using libintl.a cannot be linked statically.  */
#if !defined _LIBC
# define _nl_default_default_domain _nl_default_default_domain__
# define _nl_current_default_domain _nl_current_default_domain__
# define _nl_default_dirname _nl_default_dirname__
# define _nl_domain_bindings _nl_domain_bindings__
#endif

/* Some compilers, like SunOS4 cc, don't have offsetof in <stddef.h>.  */
#ifndef offsetof
# define offsetof(type,ident) ((size_t)&(((type*)0)->ident))
#endif

/* @@ end of prolog @@ */

#define HAVE_STPCPY 1

#ifdef _GLIBC
/* Rename the non ANSI C functions.  This is required by the standard
   because some ANSI C functions will require linking with this object
   file and the name space must not be polluted.  */
# define getcwd __getcwd
# ifndef stpcpy
#  define stpcpy __stpcpy
# endif
# define tfind __tfind
#else
# if !defined HAVE_GETCWD
char *getwd ();
#  define getcwd(buf, max) getwd (buf)
# else
char *getcwd ();
# endif
# ifndef HAVE_STPCPY
static char *stpcpy PARAMS ((char *dest, const char *src));
# endif
# ifndef HAVE_MEMPCPY
static void *mempcpy PARAMS ((void *dest, const void *src, size_t n));
# endif
#endif

/* Amount to increase buffer size by in each try.  */
#define PATH_INCR 32

/* The following is from pathmax.h.  */
/* Non-POSIX BSD systems might have gcc's limits.h, which doesn't define
   PATH_MAX but might cause redefinition warnings when sys/param.h is
   later included (as on MORE/BSD 4.3).  */
#if defined _POSIX_VERSION || (defined HAVE_LIMITS_H && !defined __GNUC__)
# include <limits.h>
#endif

#ifndef _POSIX_PATH_MAX
# define _POSIX_PATH_MAX 255
#endif

#if !defined PATH_MAX && defined _PC_PATH_MAX
# define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf ("/", _PC_PATH_MAX))
#endif

/* Don't include sys/param.h if it already has been.  */
#if defined HAVE_SYS_PARAM_H && !defined PATH_MAX && !defined MAXPATHLEN
# include <sys/param.h>
#endif

#if !defined PATH_MAX && defined MAXPATHLEN
# define PATH_MAX MAXPATHLEN
#endif

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

/* XPG3 defines the result of `setlocale (category, NULL)' as:
   ``Directs `setlocale()' to query `category' and return the current
     setting of `local'.''
   However it does not specify the exact format.  Neither do SUSV2 and
   ISO C 99.  So we can use this feature only on selected systems (e.g.
   those using GNU C Library).  */
#ifdef _LIBC
# define HAVE_LOCALE_NULL
#endif

/* This is the type used for the search tree where known translations
   are stored.  */
struct known_translation_t
{
  /* Domain in which to search.  */
  char *domainname;

  /* The category.  */
  int category;

  /* State of the catalog counter at the point the string was found.  */
  int counter;

  /* Catalog where the string was found.  */
  struct loaded_l10nfile *domain;

  /* And finally the translation.  */
  const char *translation;
  size_t translation_length;

  /* Pointer to the string in question.  */
  char msgid[ZERO];
};

/* Root of the search tree with known translations.  We can use this
   only if the system provides the `tsearch' function family.  */
#if defined HAVE_TSEARCH || defined _LIBC
# include <search.h>

static void *root;

# ifdef _GLIBC
#  define tsearch __tsearch
# endif

/* Function to compare two entries in the table of known translations.  */
static int transcmp PARAMS ((const void *p1, const void *p2));
static int
transcmp (p1, p2)
     const void *p1;
     const void *p2;
{
  const struct known_translation_t *s1;
  const struct known_translation_t *s2;
  int result;

  s1 = (const struct known_translation_t *) p1;
  s2 = (const struct known_translation_t *) p2;

  result = strcmp (s1->msgid, s2->msgid);
  if (result == 0)
    {
      result = strcmp (s1->domainname, s2->domainname);
      if (result == 0)
	/* We compare the category last (though this is the cheapest
	   operation) since it is hopefully always the same (namely
	   LC_MESSAGES).  */
	result = s1->category - s2->category;
    }

  return result;
}
#endif

/* Name of the default domain used for gettext(3) prior any call to
   textdomain(3).  The default value for this is "messages".  */
const char _nl_default_default_domain[] = "messages";

/* Value used as the default domain for gettext(3).  */
const char *_nl_current_default_domain = _nl_default_default_domain;

/* Contains the default location of the message catalogs.  */
const char _nl_default_dirname[] = LOCALEDIR;

/* List with bindings of specific domains created by bindtextdomain()
   calls.  */
struct binding *_nl_domain_bindings;

/* Prototypes for local functions.  */
static char *plural_lookup PARAMS ((struct loaded_l10nfile *domain,
				    unsigned long int n,
				    const char *translation,
				    size_t translation_len))
     internal_function;
static unsigned long int plural_eval PARAMS ((struct expression *pexp,
					      unsigned long int n))
     internal_function;
static const char *category_to_name PARAMS ((int category)) internal_function;
static const char *guess_category_value PARAMS ((int category,
						 const char *categoryname))
     internal_function;


/* For those loosing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
/* Nothing has to be done.  */
# define ADD_BLOCK(list, address) /* nothing */
# define FREE_BLOCKS(list) /* nothing */
#else
struct block_list
{
  void *address;
  struct block_list *next;
};
# define ADD_BLOCK(list, addr)						      \
  do {									      \
    struct block_list *newp = (struct block_list *) malloc (sizeof (*newp));  \
    /* If we cannot get a free block we cannot add the new element to	      \
       the list.  */							      \
    if (newp != NULL) {							      \
      newp->address = (addr);						      \
      newp->next = (list);						      \
      (list) = newp;							      \
    }									      \
  } while (0)
# define FREE_BLOCKS(list)						      \
  do {									      \
    while (list != NULL) {						      \
      struct block_list *old = list;					      \
      list = list->next;						      \
      free (old);							      \
    }									      \
  } while (0)
# undef alloca
# define alloca(size) (malloc (size))
#endif	/* have alloca */


#ifdef _LIBC
/* List of blocks allocated for translations.  */
typedef struct transmem_list
{
  struct transmem_list *next;
  char data[ZERO];
} transmem_block_t;
static struct transmem_list *transmem_list;
#else
typedef unsigned char transmem_block_t;
#endif


/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define DCIGETTEXT __dcigettext
#else
# define DCIGETTEXT dcigettext__
#endif

/* Lock variable to protect the global data in the gettext implementation.  */
__libc_rwlock_define_initialized (, _nl_state_lock)

/* Checking whether the binaries runs SUID must be done and glibc provides
   easier methods therefore we make a difference here.  */
#ifdef _LIBC
# define ENABLE_SECURE __libc_enable_secure
# define DETERMINE_SECURE
#else
static int enable_secure;
# define ENABLE_SECURE (enable_secure == 1)
# define DETERMINE_SECURE \
  if (enable_secure == 0)						      \
    {									      \
      if (getuid () != geteuid () || getgid () != getegid ())		      \
	enable_secure = 1;						      \
      else								      \
	enable_secure = -1;						      \
    }
#endif

/* Look up MSGID in the DOMAINNAME message catalog for the current
   CATEGORY locale and, if PLURAL is nonzero, search over string
   depending on the plural form determined by N.  */
char *
DCIGETTEXT (domainname, msgid1, msgid2, plural, n, category)
     const char *domainname;
     const char *msgid1;
     const char *msgid2;
     int plural;
     unsigned long int n;
     int category;
{
#ifndef HAVE_ALLOCA
  struct block_list *block_list = NULL;
#endif
  struct loaded_l10nfile *domain;
  struct binding *binding;
  const char *categoryname;
  const char *categoryvalue;
  char *dirname, *xdomainname;
  char *single_locale;
  char *retval;
  size_t retlen;
  int saved_errno;
#if defined HAVE_TSEARCH || defined _LIBC
  struct known_translation_t *search;
  struct known_translation_t **foundp = NULL;
  size_t msgid_len;
#endif
  size_t domainname_len;

  /* If no real MSGID is given return NULL.  */
  if (msgid1 == NULL)
    return NULL;

  __libc_rwlock_rdlock (_nl_state_lock);

  /* If DOMAINNAME is NULL, we are interested in the default domain.  If
     CATEGORY is not LC_MESSAGES this might not make much sense but the
     definition left this undefined.  */
  if (domainname == NULL)
    domainname = _nl_current_default_domain;

#if defined HAVE_TSEARCH || defined _LIBC
  msgid_len = strlen (msgid1) + 1;

  /* Try to find the translation among those which we found at
     some time.  */
  search = (struct known_translation_t *)
	   alloca (offsetof (struct known_translation_t, msgid) + msgid_len);
  memcpy (search->msgid, msgid1, msgid_len);
  search->domainname = (char *) domainname;
  search->category = category;

  foundp = (struct known_translation_t **) tfind (search, &root, transcmp);
  if (foundp != NULL && (*foundp)->counter == _nl_msg_cat_cntr)
    {
      /* Now deal with plural.  */
      if (plural)
	retval = plural_lookup ((*foundp)->domain, n, (*foundp)->translation,
				(*foundp)->translation_length);
      else
	retval = (char *) (*foundp)->translation;

      __libc_rwlock_unlock (_nl_state_lock);
      return retval;
    }
#endif

  /* Preserve the `errno' value.  */
  saved_errno = errno;

  /* See whether this is a SUID binary or not.  */
  DETERMINE_SECURE;

  /* First find matching binding.  */
  for (binding = _nl_domain_bindings; binding != NULL; binding = binding->next)
    {
      int compare = strcmp (domainname, binding->domainname);
      if (compare == 0)
	/* We found it!  */
	break;
      if (compare < 0)
	{
	  /* It is not in the list.  */
	  binding = NULL;
	  break;
	}
    }

  if (binding == NULL)
    dirname = (char *) _nl_default_dirname;
  else if (binding->dirname[0] == '/')
    dirname = binding->dirname;
  else
    {
      /* We have a relative path.  Make it absolute now.  */
      size_t dirname_len = strlen (binding->dirname) + 1;
      size_t path_max;
      char *ret;

      path_max = (unsigned int) PATH_MAX;
      path_max += 2;		/* The getcwd docs say to do this.  */

      dirname = (char *) alloca (path_max + dirname_len);
      ADD_BLOCK (block_list, dirname);

      __set_errno (0);
      while ((ret = getcwd (dirname, path_max)) == NULL && errno == ERANGE)
	{
	  path_max += PATH_INCR;
	  dirname = (char *) alloca (path_max + dirname_len);
	  ADD_BLOCK (block_list, dirname);
	  __set_errno (0);
	}

      if (ret == NULL)
	{
	  /* We cannot get the current working directory.  Don't signal an
	     error but simply return the default string.  */
	  FREE_BLOCKS (block_list);
	  __libc_rwlock_unlock (_nl_state_lock);
	  __set_errno (saved_errno);
	  return (plural == 0
		  ? (char *) msgid1
		  /* Use the Germanic plural rule.  */
		  : n == 1 ? (char *) msgid1 : (char *) msgid2);
	}

      stpcpy (stpcpy (strchr (dirname, '\0'), "/"), binding->dirname);
    }

  /* Now determine the symbolic name of CATEGORY and its value.  */
  categoryname = category_to_name (category);
  categoryvalue = guess_category_value (category, categoryname);

  domainname_len = strlen (domainname);
  xdomainname = (char *) alloca (strlen (categoryname)
				 + domainname_len + 5);
  ADD_BLOCK (block_list, xdomainname);

  stpcpy (mempcpy (stpcpy (stpcpy (xdomainname, categoryname), "/"),
		  domainname, domainname_len),
	  ".mo");

  /* Creating working area.  */
  single_locale = (char *) alloca (strlen (categoryvalue) + 1);
  ADD_BLOCK (block_list, single_locale);


  /* Search for the given string.  This is a loop because we perhaps
     got an ordered list of languages to consider for the translation.  */
  while (1)
    {
      /* Make CATEGORYVALUE point to the next element of the list.  */
      while (categoryvalue[0] != '\0' && categoryvalue[0] == ':')
	++categoryvalue;
      if (categoryvalue[0] == '\0')
	{
	  /* The whole contents of CATEGORYVALUE has been searched but
	     no valid entry has been found.  We solve this situation
	     by implicitly appending a "C" entry, i.e. no translation
	     will take place.  */
	  single_locale[0] = 'C';
	  single_locale[1] = '\0';
	}
      else
	{
	  char *cp = single_locale;
	  while (categoryvalue[0] != '\0' && categoryvalue[0] != ':')
	    *cp++ = *categoryvalue++;
	  *cp = '\0';

	  /* When this is a SUID binary we must not allow accessing files
	     outside the dedicated directories.  */
	  if (ENABLE_SECURE && strchr (single_locale, '/') != NULL)
	    /* Ingore this entry.  */
	    continue;
	}

      /* If the current locale value is C (or POSIX) we don't load a
	 domain.  Return the MSGID.  */
      if (strcmp (single_locale, "C") == 0
	  || strcmp (single_locale, "POSIX") == 0)
	{
	  FREE_BLOCKS (block_list);
	  __libc_rwlock_unlock (_nl_state_lock);
	  __set_errno (saved_errno);
	  return (plural == 0
		  ? (char *) msgid1
		  /* Use the Germanic plural rule.  */
		  : n == 1 ? (char *) msgid1 : (char *) msgid2);
	}


      /* Find structure describing the message catalog matching the
	 DOMAINNAME and CATEGORY.  */
      domain = _nl_find_domain (dirname, single_locale, xdomainname, binding);

      if (domain != NULL)
	{
	  retval = _nl_find_msg (domain, binding, msgid1, &retlen);

	  if (retval == NULL)
	    {
	      int cnt;

	      for (cnt = 0; domain->successor[cnt] != NULL; ++cnt)
		{
		  retval = _nl_find_msg (domain->successor[cnt], binding,
					 msgid1, &retlen);

		  if (retval != NULL)
		    {
		      domain = domain->successor[cnt];
		      break;
		    }
		}
	    }

	  if (retval != NULL)
	    {
	      /* Found the translation of MSGID1 in domain DOMAIN:
		 starting at RETVAL, RETLEN bytes.  */
	      FREE_BLOCKS (block_list);
	      __set_errno (saved_errno);
#if defined HAVE_TSEARCH || defined _LIBC
	      if (foundp == NULL)
		{
		  /* Create a new entry and add it to the search tree.  */
		  struct known_translation_t *newp;

		  newp = (struct known_translation_t *)
		    malloc (offsetof (struct known_translation_t, msgid)
			    + msgid_len + domainname_len + 1);
		  if (newp != NULL)
		    {
		      newp->domainname =
			mempcpy (newp->msgid, msgid1, msgid_len);
		      memcpy (newp->domainname, domainname, domainname_len + 1);
		      newp->category = category;
		      newp->counter = _nl_msg_cat_cntr;
		      newp->domain = domain;
		      newp->translation = retval;
		      newp->translation_length = retlen;

		      /* Insert the entry in the search tree.  */
		      foundp = (struct known_translation_t **)
			tsearch (newp, &root, transcmp);
		      if (foundp == NULL
			  || __builtin_expect (*foundp != newp, 0))
			/* The insert failed.  */
			free (newp);
		    }
		}
	      else
		{
		  /* We can update the existing entry.  */
		  (*foundp)->counter = _nl_msg_cat_cntr;
		  (*foundp)->domain = domain;
		  (*foundp)->translation = retval;
		  (*foundp)->translation_length = retlen;
		}
#endif
	      /* Now deal with plural.  */
	      if (plural)
		retval = plural_lookup (domain, n, retval, retlen);

	      __libc_rwlock_unlock (_nl_state_lock);
	      return retval;
	    }
	}
    }
  /* NOTREACHED */
}


char *
internal_function
_nl_find_msg (domain_file, domainbinding, msgid, lengthp)
     struct loaded_l10nfile *domain_file;
     struct binding *domainbinding;
     const char *msgid;
     size_t *lengthp;
{
  struct loaded_domain *domain;
  size_t act;
  char *result;
  size_t resultlen;

  if (domain_file->decided == 0)
    _nl_load_domain (domain_file, domainbinding);

  if (domain_file->data == NULL)
    return NULL;

  domain = (struct loaded_domain *) domain_file->data;

  /* Locate the MSGID and its translation.  */
  if (domain->hash_size > 2 && domain->hash_tab != NULL)
    {
      /* Use the hashing table.  */
      nls_uint32 len = strlen (msgid);
      nls_uint32 hash_val = hash_string (msgid);
      nls_uint32 idx = hash_val % domain->hash_size;
      nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));

      while (1)
	{
	  nls_uint32 nstr = W (domain->must_swap, domain->hash_tab[idx]);

	  if (nstr == 0)
	    /* Hash table entry is empty.  */
	    return NULL;

	  /* Compare msgid with the original string at index nstr-1.
	     We compare the lengths with >=, not ==, because plural entries
	     are represented by strings with an embedded NUL.  */
	  if (W (domain->must_swap, domain->orig_tab[nstr - 1].length) >= len
	      && (strcmp (msgid,
			  domain->data + W (domain->must_swap,
					    domain->orig_tab[nstr - 1].offset))
		  == 0))
	    {
	      act = nstr - 1;
	      goto found;
	    }

	  if (idx >= domain->hash_size - incr)
	    idx -= domain->hash_size - incr;
	  else
	    idx += incr;
	}
      /* NOTREACHED */
    }
  else
    {
      /* Try the default method:  binary search in the sorted array of
	 messages.  */
      size_t top, bottom;

      bottom = 0;
      top = domain->nstrings;
      while (bottom < top)
	{
	  int cmp_val;

	  act = (bottom + top) / 2;
	  cmp_val = strcmp (msgid, (domain->data
				    + W (domain->must_swap,
					 domain->orig_tab[act].offset)));
	  if (cmp_val < 0)
	    top = act;
	  else if (cmp_val > 0)
	    bottom = act + 1;
	  else
	    goto found;
	}
      /* No translation was found.  */
      return NULL;
    }

 found:
  /* The translation was found at index ACT.  If we have to convert the
     string to use a different character set, this is the time.  */
  result = ((char *) domain->data
	    + W (domain->must_swap, domain->trans_tab[act].offset));
  resultlen = W (domain->must_swap, domain->trans_tab[act].length) + 1;

#if defined _LIBC || HAVE_ICONV
  if (domain->codeset_cntr
      != (domainbinding != NULL ? domainbinding->codeset_cntr : 0))
    {
      /* The domain's codeset has changed through bind_textdomain_codeset()
	 since the message catalog was initialized or last accessed.  We
	 have to reinitialize the converter.  */
      _nl_free_domain_conv (domain);
      _nl_init_domain_conv (domain_file, domain, domainbinding);
    }

  if (
# ifdef _GLIBC
      domain->conv != (__gconv_t) -1
# else
#  if HAVE_ICONV
      domain->conv != (iconv_t) -1
#  endif
# endif
      )
    {
      /* We are supposed to do a conversion.  First allocate an
	 appropriate table with the same structure as the table
	 of translations in the file, where we can put the pointers
	 to the converted strings in.
	 There is a slight complication with plural entries.  They
	 are represented by consecutive NUL terminated strings.  We
	 handle this case by converting RESULTLEN bytes, including
	 NULs.  */

      if (domain->conv_tab == NULL
	  && ((domain->conv_tab = (char **) calloc (domain->nstrings,
						    sizeof (char *)))
	      == NULL))
	/* Mark that we didn't succeed allocating a table.  */
	domain->conv_tab = (char **) -1;

      if (__builtin_expect (domain->conv_tab == (char **) -1, 0))
	/* Nothing we can do, no more memory.  */
	goto converted;

      if (domain->conv_tab[act] == NULL)
	{
	  /* We haven't used this string so far, so it is not
	     translated yet.  Do this now.  */
	  /* We use a bit more efficient memory handling.
	     We allocate always larger blocks which get used over
	     time.  This is faster than many small allocations.   */
	  __libc_lock_define_initialized (static, lock)
# define INITIAL_BLOCK_SIZE	4080
	  static unsigned char *freemem;
	  static size_t freemem_size;

	  const unsigned char *inbuf;
	  unsigned char *outbuf;
	  int malloc_count;
# ifndef _LIBC
	  transmem_block_t *transmem_list = NULL;
# endif

	  __libc_lock_lock (lock);

	  inbuf = (const unsigned char *) result;
	  outbuf = freemem + sizeof (size_t);

	  malloc_count = 0;
	  while (1)
	    {
	      transmem_block_t *newmem;
# ifdef _GLIBC
	      size_t non_reversible;
	      int res;

	      if (freemem_size < sizeof (size_t))
		goto resize_freemem;

	      res = __gconv (domain->conv,
			     &inbuf, inbuf + resultlen,
			     &outbuf,
			     outbuf + freemem_size - sizeof (size_t),
			     &non_reversible);

	      if (res == __GCONV_OK || res == __GCONV_EMPTY_INPUT)
		break;

	      if (res != __GCONV_FULL_OUTPUT)
		{
		  __libc_lock_unlock (lock);
		  goto converted;
		}

	      inbuf = result;
# else
#  if HAVE_ICONV
	      const char *inptr = (const char *) inbuf;
	      size_t inleft = resultlen;
	      char *outptr = (char *) outbuf;
	      size_t outleft;

	      if (freemem_size < sizeof (size_t))
		goto resize_freemem;

	      outleft = freemem_size - sizeof (size_t);
	      if (iconv (domain->conv,
			 (char **) &inptr, &inleft,
			 &outptr, &outleft)
		  != (size_t) (-1))
		{
		  outbuf = (unsigned char *) outptr;
		  break;
		}
	      if (errno != E2BIG)
		{
		  __libc_lock_unlock (lock);
		  goto converted;
		}
#  endif
# endif

	    resize_freemem:
	      /* We must allocate a new buffer or resize the old one.  */
	      if (malloc_count > 0)
		{
		  ++malloc_count;
		  freemem_size = malloc_count * INITIAL_BLOCK_SIZE;
		  newmem = (transmem_block_t *) realloc (transmem_list,
							 freemem_size);
# ifdef _LIBC
		  if (newmem != NULL)
		    transmem_list = transmem_list->next;
		  else
		    {
		      struct transmem_list *old = transmem_list;

		      transmem_list = transmem_list->next;
		      free (old);
		    }
# endif
		}
	      else
		{
		  malloc_count = 1;
		  freemem_size = INITIAL_BLOCK_SIZE;
		  newmem = (transmem_block_t *) malloc (freemem_size);
		}
	      if (__builtin_expect (newmem == NULL, 0))
		{
		  freemem = NULL;
		  freemem_size = 0;
		  __libc_lock_unlock (lock);
		  goto converted;
		}

# ifdef _LIBC
	      /* Add the block to the list of blocks we have to free
                 at some point.  */
	      newmem->next = transmem_list;
	      transmem_list = newmem;

	      freemem = newmem->data;
	      freemem_size -= offsetof (struct transmem_list, data);
# else
	      transmem_list = newmem;
	      freemem = newmem;
# endif

	      outbuf = freemem + sizeof (size_t);
	    }

	  /* We have now in our buffer a converted string.  Put this
	     into the table of conversions.  */
	  *(size_t *) freemem = outbuf - freemem - sizeof (size_t);
	  domain->conv_tab[act] = (char *) freemem;
	  /* Shrink freemem, but keep it aligned.  */
	  freemem_size -= outbuf - freemem;
	  freemem = outbuf;
	  freemem += freemem_size & (alignof (size_t) - 1);
	  freemem_size = freemem_size & ~ (alignof (size_t) - 1);

	  __libc_lock_unlock (lock);
	}

      /* Now domain->conv_tab[act] contains the translation of all
	 the plural variants.  */
      result = domain->conv_tab[act] + sizeof (size_t);
      resultlen = *(size_t *) domain->conv_tab[act];
    }

 converted:
  /* The result string is converted.  */

#endif /* _LIBC || HAVE_ICONV */

  *lengthp = resultlen;
  return result;
}


/* Look up a plural variant.  */
static char *
internal_function
plural_lookup (domain, n, translation, translation_len)
     struct loaded_l10nfile *domain;
     unsigned long int n;
     const char *translation;
     size_t translation_len;
{
  struct loaded_domain *domaindata = (struct loaded_domain *) domain->data;
  unsigned long int index;
  const char *p;

  index = plural_eval (domaindata->plural, n);
  if (index >= domaindata->nplurals)
    /* This should never happen.  It means the plural expression and the
       given maximum value do not match.  */
    index = 0;

  /* Skip INDEX strings at TRANSLATION.  */
  p = translation;
  while (index-- > 0)
    {
#ifdef _GLIBC
      p = __rawmemchr (p, '\0');
#else
      p = strchr (p, '\0');
#endif
      /* And skip over the NUL byte.  */
      p++;

      if (p >= translation + translation_len)
	/* This should never happen.  It means the plural expression
	   evaluated to a value larger than the number of variants
	   available for MSGID1.  */
	return (char *) translation;
    }
  return (char *) p;
}


/* Function to evaluate the plural expression and return an index value.  */
static unsigned long int
internal_function
plural_eval (pexp, n)
     struct expression *pexp;
     unsigned long int n;
{
  switch (pexp->nargs)
    {
    case 0:
      switch (pexp->operation)
	{
	case var:
	  return n;
	case num:
	  return pexp->val.num;
	default:
	  break;
	}
      /* NOTREACHED */
      break;
    case 1:
      {
	/* pexp->operation must be lnot.  */
	unsigned long int arg = plural_eval (pexp->val.args[0], n);
	return ! arg;
      }
    case 2:
      {
	unsigned long int leftarg = plural_eval (pexp->val.args[0], n);
	if (pexp->operation == lor)
	  return leftarg || plural_eval (pexp->val.args[1], n);
	else if (pexp->operation == land)
	  return leftarg && plural_eval (pexp->val.args[1], n);
	else
	  {
	    unsigned long int rightarg = plural_eval (pexp->val.args[1], n);

	    switch (pexp->operation)
	      {
	      case mult:
		return leftarg * rightarg;
	      case divide:
		return leftarg / rightarg;
	      case module:
		return leftarg % rightarg;
	      case plus:
		return leftarg + rightarg;
	      case minus:
		return leftarg - rightarg;
	      case less_than:
		return leftarg < rightarg;
	      case greater_than:
		return leftarg > rightarg;
	      case less_or_equal:
		return leftarg <= rightarg;
	      case greater_or_equal:
		return leftarg >= rightarg;
	      case equal:
		return leftarg == rightarg;
	      case not_equal:
		return leftarg != rightarg;
	      default:
		break;
	      }
	  }
	/* NOTREACHED */
	break;
      }
    case 3:
      {
	/* pexp->operation must be qmop.  */
	unsigned long int boolarg = plural_eval (pexp->val.args[0], n);
	return plural_eval (pexp->val.args[boolarg ? 1 : 2], n);
      }
    }
  /* NOTREACHED */
  return 0;
}


/* Return string representation of locale CATEGORY.  */
static const char *
internal_function
category_to_name (category)
     int category;
{
  const char *retval;

  switch (category)
  {
#ifdef LC_COLLATE
  case LC_COLLATE:
    retval = "LC_COLLATE";
    break;
#endif
#ifdef LC_CTYPE
  case LC_CTYPE:
    retval = "LC_CTYPE";
    break;
#endif
#ifdef LC_MONETARY
  case LC_MONETARY:
    retval = "LC_MONETARY";
    break;
#endif
#ifdef LC_NUMERIC
  case LC_NUMERIC:
    retval = "LC_NUMERIC";
    break;
#endif
#ifdef LC_TIME
  case LC_TIME:
    retval = "LC_TIME";
    break;
#endif
#ifdef LC_MESSAGES
  case LC_MESSAGES:
    retval = "LC_MESSAGES";
    break;
#endif
#ifdef LC_RESPONSE
  case LC_RESPONSE:
    retval = "LC_RESPONSE";
    break;
#endif
#ifdef LC_ALL
  case LC_ALL:
    /* This might not make sense but is perhaps better than any other
       value.  */
    retval = "LC_ALL";
    break;
#endif
  default:
    /* If you have a better idea for a default value let me know.  */
    retval = "LC_XXX";
  }

  return retval;
}

/* Guess value of current locale from value of the environment variables.  */
static const char *
internal_function
guess_category_value (category, categoryname)
     int category;
     const char *categoryname;
{
  const char *language;
  const char *retval;

  /* The highest priority value is the `LANGUAGE' environment
     variable.  But we don't use the value if the currently selected
     locale is the C locale.  This is a GNU extension.  */
  language = getenv ("LANGUAGE");
  if (language != NULL && language[0] == '\0')
    language = NULL;

  /* We have to proceed with the POSIX methods of looking to `LC_ALL',
     `LC_xxx', and `LANG'.  On some systems this can be done by the
     `setlocale' function itself.  */
#if defined _LIBC || (defined HAVE_SETLOCALE && defined HAVE_LC_MESSAGES && defined HAVE_LOCALE_NULL)
  retval = setlocale (category, NULL);
#else
  /* Setting of LC_ALL overwrites all other.  */
  retval = getenv ("LC_ALL");
  if (retval == NULL || retval[0] == '\0')
    {
      /* Next comes the name of the desired category.  */
      retval = getenv (categoryname);
      if (retval == NULL || retval[0] == '\0')
	{
	  /* Last possibility is the LANG environment variable.  */
	  retval = getenv ("LANG");
	  if (retval == NULL || retval[0] == '\0')
	    /* We use C as the default domain.  POSIX says this is
	       implementation defined.  */
	    return "C";
	}
    }
#endif

  return language != NULL && strcmp (retval, "C") != 0 ? language : retval;
}

/* @@ begin of epilog @@ */

/* We don't want libintl.a to depend on any other library.  So we
   avoid the non-standard function stpcpy.  In GNU C Library this
   function is available, though.  Also allow the symbol HAVE_STPCPY
   to be defined.  */
#if !_GLIBC && !HAVE_STPCPY
static char *
stpcpy (dest, src)
     char *dest;
     const char *src;
{
  while ((*dest++ = *src++) != '\0')
    /* Do nothing. */ ;
  return dest - 1;
}
#endif

#if !_LIBC && !HAVE_MEMPCPY
static void *
mempcpy (dest, src, n)
     void *dest;
     const void *src;
     size_t n;
{
  return (void *) ((char *) memcpy (dest, src, n) + n);
}
#endif


#ifdef _LIBC
/* If we want to free all resources we have to do some work at
   program's end.  */
static void __attribute__ ((unused))
free_mem (void)
{
  void *old;

  while (_nl_domain_bindings != NULL)
    {
      struct binding *oldp = _nl_domain_bindings;
      _nl_domain_bindings = _nl_domain_bindings->next;
      if (oldp->dirname != _nl_default_dirname)
	/* Yes, this is a pointer comparison.  */
	free (oldp->dirname);
      free (oldp->codeset);
      free (oldp);
    }

  if (_nl_current_default_domain != _nl_default_default_domain)
    /* Yes, again a pointer comparison.  */
    free ((char *) _nl_current_default_domain);

  /* Remove the search tree with the known translations.  */
  tdestroy (root, free);
  root = NULL;

  while (transmem_list != NULL)
    {
      old = transmem_list;
      transmem_list = transmem_list->next;
      free (old);
    }
}

text_set_element (__libc_subfreeres, free_mem);
#endif
#else  /* !_MB_CAPABLE */

#include <machine/weakalias.h>
#include <sys/lock.h>

/* Name of the default domain used for gettext(3) prior any call to
   textdomain(3).  The default value for this is "messages".  */
const char _nl_default_default_domain[] = "messages";

/* Value used as the default domain for gettext(3).  */
const char *_nl_current_default_domain = _nl_default_default_domain;

/* Contains the default location of the message catalogs.  */
const char _nl_default_dirname[] = LOCALEDIR;

/* List with bindings of specific domains created by bindtextdomain()
   calls.  */
struct binding *_nl_domain_bindings;

/* this is actually usually defined in loadmsgcat.c, but we do it
   here so we don't need the other file at all. */
int _nl_msg_cat_cntr;

/* Lock variable to protect the global data in the gettext implementation.  */
__libc_rwlock_define_initialized (, _nl_state_lock)


/* For non-MB-capable programs, the locale must be "C", so we can
   avoid excess program size by short-circuiting this function. */
char *
__dcigettext (domainname, msgid1, msgid2, plural, n, category)
     const char *domainname;
     const char *msgid1;
     const char *msgid2;
     int plural;
     unsigned long int n;
     int category;
{
  if (plural && n > 1)
    return msgid2;
  return msgid1;
}
weak_alias(__dcigettext, dcigettext)

#endif /* !_MB_CAPABLE */
