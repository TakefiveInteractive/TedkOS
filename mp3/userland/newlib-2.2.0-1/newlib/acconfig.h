#ifndef __NEWLIB_H__

#define __NEWLIB_H__ 1
@TOP@

/* EL/IX level */
#undef _ELIX_LEVEL

/* Newlib version */
#undef _NEWLIB_VERSION

/* C99 formats support (such as %a, %zu, ...) in IO functions like
 * printf/scanf enabled */
#undef _WANT_IO_C99_FORMATS

/* long long type support in IO functions like printf/scanf enabled */
#undef _WANT_IO_LONG_LONG

/* long double type support in IO functions like printf/scanf enabled */
#undef _WANT_IO_LONG_DOUBLE

/* Positional argument support in printf functions enabled.  */
#undef _WANT_IO_POS_ARGS

/* Optional reentrant struct support.  Used mostly on platforms with
   very restricted storage.  */
#undef _WANT_REENT_SMALL

/* Multibyte supported */
#undef _MB_CAPABLE

/* MB_LEN_MAX */
#undef _MB_LEN_MAX

/* ICONV enabled */
#undef _ICONV_ENABLED

/* Enable ICONV external CCS files loading capabilities */
#undef _ICONV_ENABLE_EXTERNAL_CCS

/* Define if the linker supports .preinit_array/.init_array/.fini_array
 * sections.  */
#undef  HAVE_INITFINI_ARRAY

/* True if atexit() may dynamically allocate space for cleanup
   functions.  */
#undef  _ATEXIT_DYNAMIC_ALLOC

/* Define if the compiler supports aliasing an array to an address.  */
#undef  _HAVE_ARRAY_ALIASING

/* Define if the platform supports long double type.  */
#undef  _HAVE_LONG_DOUBLE

/* Define if compiler supports -fno-tree-loop-distribute-patterns. */
#undef  _HAVE_CC_INHIBIT_LOOP_TO_LIBCALL

/* Define if the platform long double type is equal to double.  */
#undef  _LDBL_EQ_DBL
 
/* Define if ivo supported in streamio.  */
#undef _FVWRITE_IN_STREAMIO

/* Define if fseek functions support seek optimization.  */
#undef _FSEEK_OPTIMIZATION

/* Define if wide char orientation is supported.  */
#undef  _WIDE_ORIENT

/* Define if unbuffered stream file optimization is supported.  */
#undef _UNBUF_STREAM_OPT

/* Define if enable lite version of exit.  */
#undef _LITE_EXIT

/* Define if nano version formatted IO is used.  */
#undef _NANO_FORMATTED_IO

@BOTTOM@
/*
 * Iconv encodings enabled ("to" direction)
 */
#undef _ICONV_TO_ENCODING_BIG5
#undef _ICONV_TO_ENCODING_CP775
#undef _ICONV_TO_ENCODING_CP850
#undef _ICONV_TO_ENCODING_CP852
#undef _ICONV_TO_ENCODING_CP855
#undef _ICONV_TO_ENCODING_CP866
#undef _ICONV_TO_ENCODING_EUC_JP
#undef _ICONV_TO_ENCODING_EUC_TW
#undef _ICONV_TO_ENCODING_EUC_KR
#undef _ICONV_TO_ENCODING_ISO_8859_1
#undef _ICONV_TO_ENCODING_ISO_8859_10
#undef _ICONV_TO_ENCODING_ISO_8859_11
#undef _ICONV_TO_ENCODING_ISO_8859_13
#undef _ICONV_TO_ENCODING_ISO_8859_14
#undef _ICONV_TO_ENCODING_ISO_8859_15
#undef _ICONV_TO_ENCODING_ISO_8859_2
#undef _ICONV_TO_ENCODING_ISO_8859_3
#undef _ICONV_TO_ENCODING_ISO_8859_4
#undef _ICONV_TO_ENCODING_ISO_8859_5
#undef _ICONV_TO_ENCODING_ISO_8859_6
#undef _ICONV_TO_ENCODING_ISO_8859_7
#undef _ICONV_TO_ENCODING_ISO_8859_8
#undef _ICONV_TO_ENCODING_ISO_8859_9
#undef _ICONV_TO_ENCODING_ISO_IR_111
#undef _ICONV_TO_ENCODING_KOI8_R
#undef _ICONV_TO_ENCODING_KOI8_RU
#undef _ICONV_TO_ENCODING_KOI8_U
#undef _ICONV_TO_ENCODING_KOI8_UNI
#undef _ICONV_TO_ENCODING_UCS_2
#undef _ICONV_TO_ENCODING_UCS_2_INTERNAL
#undef _ICONV_TO_ENCODING_UCS_2BE
#undef _ICONV_TO_ENCODING_UCS_2LE
#undef _ICONV_TO_ENCODING_UCS_4
#undef _ICONV_TO_ENCODING_UCS_4_INTERNAL
#undef _ICONV_TO_ENCODING_UCS_4BE
#undef _ICONV_TO_ENCODING_UCS_4LE
#undef _ICONV_TO_ENCODING_US_ASCII
#undef _ICONV_TO_ENCODING_UTF_16
#undef _ICONV_TO_ENCODING_UTF_16BE
#undef _ICONV_TO_ENCODING_UTF_16LE
#undef _ICONV_TO_ENCODING_UTF_8
#undef _ICONV_TO_ENCODING_WIN_1250
#undef _ICONV_TO_ENCODING_WIN_1251
#undef _ICONV_TO_ENCODING_WIN_1252
#undef _ICONV_TO_ENCODING_WIN_1253
#undef _ICONV_TO_ENCODING_WIN_1254
#undef _ICONV_TO_ENCODING_WIN_1255
#undef _ICONV_TO_ENCODING_WIN_1256
#undef _ICONV_TO_ENCODING_WIN_1257
#undef _ICONV_TO_ENCODING_WIN_1258

/*
 * Iconv encodings enabled ("from" direction)
 */
#undef _ICONV_FROM_ENCODING_BIG5
#undef _ICONV_FROM_ENCODING_CP775
#undef _ICONV_FROM_ENCODING_CP850
#undef _ICONV_FROM_ENCODING_CP852
#undef _ICONV_FROM_ENCODING_CP855
#undef _ICONV_FROM_ENCODING_CP866
#undef _ICONV_FROM_ENCODING_EUC_JP
#undef _ICONV_FROM_ENCODING_EUC_TW
#undef _ICONV_FROM_ENCODING_EUC_KR
#undef _ICONV_FROM_ENCODING_ISO_8859_1
#undef _ICONV_FROM_ENCODING_ISO_8859_10
#undef _ICONV_FROM_ENCODING_ISO_8859_11
#undef _ICONV_FROM_ENCODING_ISO_8859_13
#undef _ICONV_FROM_ENCODING_ISO_8859_14
#undef _ICONV_FROM_ENCODING_ISO_8859_15
#undef _ICONV_FROM_ENCODING_ISO_8859_2
#undef _ICONV_FROM_ENCODING_ISO_8859_3
#undef _ICONV_FROM_ENCODING_ISO_8859_4
#undef _ICONV_FROM_ENCODING_ISO_8859_5
#undef _ICONV_FROM_ENCODING_ISO_8859_6
#undef _ICONV_FROM_ENCODING_ISO_8859_7
#undef _ICONV_FROM_ENCODING_ISO_8859_8
#undef _ICONV_FROM_ENCODING_ISO_8859_9
#undef _ICONV_FROM_ENCODING_ISO_IR_111
#undef _ICONV_FROM_ENCODING_KOI8_R
#undef _ICONV_FROM_ENCODING_KOI8_RU
#undef _ICONV_FROM_ENCODING_KOI8_U
#undef _ICONV_FROM_ENCODING_KOI8_UNI
#undef _ICONV_FROM_ENCODING_UCS_2
#undef _ICONV_FROM_ENCODING_UCS_2_INTERNAL
#undef _ICONV_FROM_ENCODING_UCS_2BE
#undef _ICONV_FROM_ENCODING_UCS_2LE
#undef _ICONV_FROM_ENCODING_UCS_4
#undef _ICONV_FROM_ENCODING_UCS_4_INTERNAL
#undef _ICONV_FROM_ENCODING_UCS_4BE
#undef _ICONV_FROM_ENCODING_UCS_4LE
#undef _ICONV_FROM_ENCODING_US_ASCII
#undef _ICONV_FROM_ENCODING_UTF_16
#undef _ICONV_FROM_ENCODING_UTF_16BE
#undef _ICONV_FROM_ENCODING_UTF_16LE
#undef _ICONV_FROM_ENCODING_UTF_8
#undef _ICONV_FROM_ENCODING_WIN_1250
#undef _ICONV_FROM_ENCODING_WIN_1251
#undef _ICONV_FROM_ENCODING_WIN_1252
#undef _ICONV_FROM_ENCODING_WIN_1253
#undef _ICONV_FROM_ENCODING_WIN_1254
#undef _ICONV_FROM_ENCODING_WIN_1255
#undef _ICONV_FROM_ENCODING_WIN_1256
#undef _ICONV_FROM_ENCODING_WIN_1257
#undef _ICONV_FROM_ENCODING_WIN_1258

#endif /* !__NEWLIB_H__ */
