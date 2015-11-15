/*
 * Copyright (c) 2004, 2005 by
 * Ralf Corsepius, Ulm/Germany. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
 * @todo - Add fast<N>_t types.
 * @todo - Add support for wint_t types.
 */

#ifndef _STDINT_H
#define _STDINT_H

#include <sys/types.h>
#include <bits/wordsize.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && (__GNUC__ >= 3 ) \
  && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ > 2 ) 
#define __STDINT_EXP(x) __##x##__
#else
#define __STDINT_EXP(x) x
#include <limits.h>
#endif

#if __STDINT_EXP(SCHAR_MAX) == 0x7f
#define __int8_t_defined 1
#endif

#if __int8_t_defined
typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;
#define __int_least8_t_defined 1
#endif

#if __STDINT_EXP(SHRT_MAX) == 0x7fff
#define __int16_t_defined 1
#elif __STDINT_EXP(INT_MAX) == 0x7fff
#define __int16_t_defined 1
#elif __STDINT_EXP(SCHAR_MAX) == 0x7fff
#define __int16_t_defined 1
#endif

#if __int16_t_defined
typedef int16_t   	int_least16_t;
typedef uint16_t 	uint_least16_t;
#define __int_least16_t_defined 1

#ifndef __int_least8_t_defined
typedef int16_t	   	int_least8_t;
typedef uint16_t  	uint_least8_t;
#define __int_least8_t_defined 1
#endif
#endif

#if __STDINT_EXP(INT_MAX) == 0x7fffffffL
#define __int32_t_defined 1
#elif __STDINT_EXP(LONG_MAX) == 0x7fffffffL
#define __int32_t_defined 1
#define __have_long32 1
#elif __STDINT_EXP(SHRT_MAX) == 0x7fffffffL
#define __int32_t_defined 1
#elif __STDINT_EXP(SCHAR_MAX) == 0x7fffffffL
#define __int32_t_defined 1
#endif

#if __int32_t_defined
typedef int32_t   	int_least32_t;
typedef uint32_t 	uint_least32_t;
#define __int_least32_t_defined 1

#ifndef __int_least8_t_defined
typedef int32_t	   	int_least8_t;
typedef uint32_t  	uint_least8_t;
#define __int_least8_t_defined 1
#endif

#ifndef __int_least16_t_defined
typedef int32_t	   	int_least16_t;
typedef uint32_t  	uint_least16_t;
#define __int_least16_t_defined 1
#endif
#endif

/* Fast types.  */

/* Signed.  */
typedef signed char             int_fast8_t;
#if __WORDSIZE == 64
typedef long int                int_fast16_t;
typedef long int                int_fast32_t;
typedef long int                int_fast64_t;
#else
typedef int                     int_fast16_t;
typedef int                     int_fast32_t;
__extension__
typedef long long int           int_fast64_t;
#endif

/* Unsigned.  */
typedef unsigned char           uint_fast8_t;
#if __WORDSIZE == 64
typedef unsigned long int       uint_fast16_t;
typedef unsigned long int       uint_fast32_t;
typedef unsigned long int       uint_fast64_t;
#else
typedef unsigned int            uint_fast16_t;
typedef unsigned int            uint_fast32_t;
__extension__
typedef unsigned long long int  uint_fast64_t;
#endif

#if __STDINT_EXP(LONG_MAX) > 0x7fffffff
#define __int64_t_defined 1
#define __have_long64 1
#elif  defined(__LONG_LONG_MAX__) && (__LONG_LONG_MAX__ > 0x7fffffff)
#define __int64_t_defined 1
#define __have_longlong64 1
#elif  defined(LLONG_MAX) && (LLONG_MAX > 0x7fffffff)
#define __int64_t_defined 1
#define __have_longlong64 1
#elif  __STDINT_EXP(INT_MAX) > 0x7fffffff
#define __int64_t_defined 1
#endif

#if __int64_t_defined
typedef int64_t   	int_least64_t;
typedef uint64_t 	uint_least64_t;
#define __int_least64_t_defined 1

#ifndef __int_least8_t_defined
typedef int64_t	   	int_least8_t;
typedef uint64_t  	uint_least8_t;
#define __int_least8_t_defined 1
#endif

#ifndef __int_least16_t_defined
typedef int64_t	   	int_least16_t;
typedef uint64_t  	uint_least16_t;
#define __int_least16_t_defined 1
#endif

#ifndef __int_least32_t_defined
typedef int64_t	   	int_least32_t;
typedef uint64_t  	uint_least32_t;
#define __int_least32_t_defined 1
#endif
#endif

/* Greatest-width integer types */
/* Modern GCCs provide __INTMAX_TYPE__ */
#if defined(__INTMAX_TYPE__)
  typedef __INTMAX_TYPE__ intmax_t;
#elif __have_longlong64
  typedef signed long long intmax_t;
#else
  typedef signed long intmax_t;
#endif

/* Modern GCCs provide __UINTMAX_TYPE__ */
#if defined(__UINTMAX_TYPE__)
  typedef __UINTMAX_TYPE__ uintmax_t;
#elif __have_longlong64
  typedef unsigned long long uintmax_t;
#else
  typedef unsigned long uintmax_t;
#endif
  
/* Limits of Specified-Width Integer Types */

#if __int8_t_defined
#define INT8_MIN 	-128
#define INT8_MAX 	 127
#define UINT8_MAX 	 255
#endif

#if __int_least8_t_defined
#define INT_LEAST8_MIN 	-128
#define INT_LEAST8_MAX 	 127
#define UINT_LEAST8_MAX	 255
#else
#error required type int_least8_t missing
#endif

#if __int16_t_defined
#define INT16_MIN 	-32768
#define INT16_MAX 	 32767
#define UINT16_MAX 	 65535
#endif

#if __int_least16_t_defined
#define INT_LEAST16_MIN	-32768
#define INT_LEAST16_MAX	 32767
#define UINT_LEAST16_MAX 65535
#else
#error required type int_least16_t missing
#endif

#if __int32_t_defined
#define INT32_MIN 	 (-2147483647-1)
#define INT32_MAX 	 2147483647
#define UINT32_MAX       4294967295U
#endif

#if __int_least32_t_defined
#define INT_LEAST32_MIN  (-2147483647-1)
#define INT_LEAST32_MAX  2147483647
#define UINT_LEAST32_MAX 4294967295U
#else
#error required type int_least32_t missing
#endif

#if __int64_t_defined
#ifdef __have_long64
#define INT64_MIN 	(-9223372036854775807L-1L)
#define INT64_MAX 	 9223372036854775807L
#define UINT64_MAX 	18446744073709551615U
#elif defined(__have_longlong64)
#define INT64_MIN 	(-9223372036854775807LL-1LL)
#define INT64_MAX 	 9223372036854775807LL
#define UINT64_MAX 	18446744073709551615ULL
#endif
#endif

#if __int_least64_t_defined
#ifdef __have_long64
#define INT_LEAST64_MIN  (-9223372036854775807L-1L)
#define INT_LEAST64_MAX  9223372036854775807L
#define UINT_LEAST64_MAX 18446744073709551615U
#elif defined(__have_longlong64)
#define INT_LEAST64_MIN  (-9223372036854775807LL-1LL)
#define INT_LEAST64_MAX  9223372036854775807LL
#define UINT_LEAST64_MAX 18446744073709551615ULL
#endif
#endif

/* This must match size_t in stddef.h, currently long unsigned int */
#define SIZE_MAX (__STDINT_EXP(LONG_MAX) * 2UL + 1)

/* This must match sig_atomic_t in <signal.h> (currently int) */
#define SIG_ATOMIC_MIN (-__STDINT_EXP(INT_MAX) - 1)
#define SIG_ATOMIC_MAX __STDINT_EXP(INT_MAX)

/* This must match ptrdiff_t  in <stddef.h> (currently long int) */
#define PTRDIFF_MIN (-__STDINT_EXP(LONG_MAX) - 1L)
#define PTRDIFF_MAX __STDINT_EXP(LONG_MAX)

/** Macros for minimum-width integer constant expressions */
#define INT8_C(x)	x
#define UINT8_C(x)	x##U

#define INT16_C(x)	x
#define UINT16_C(x)	x##U

#if __have_long32
#define INT32_C(x)	x##L
#define UINT32_C(x)	x##UL
#else
#define INT32_C(x)	x
#define UINT32_C(x)	x##U
#endif

#if __int64_t_defined
#if __have_longlong64
#define INT64_C(x)	x##LL
#define UINT64_C(x)	x##ULL
#else
#define INT64_C(x)	x##L
#define UINT64_C(x)	x##UL
#endif
#endif

/** Macros for greatest-width integer constant expression */
#if __have_longlong64
#define INTMAX_C(x)	x##LL
#define UINTMAX_C(x)	x##ULL
#else
#define INTMAX_C(x)	x##L
#define UINTMAX_C(x)	x##UL
#endif


#ifdef __cplusplus
}
#endif

#endif /* _STDINT_H */
