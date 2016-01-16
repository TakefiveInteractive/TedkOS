
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_CONFIG_HPP_INCLUDED
#define BOOST_TT_CONFIG_HPP_INCLUDED

#ifndef BOOST_CONFIG_HPP
#include <boost/config.hpp>
#endif
#include <boost/version.hpp>

//
// whenever we have a conversion function with ellipses
// it needs to be declared __cdecl to suppress compiler
// warnings from MS and Borland compilers (this *must*
// appear before we include is_same.hpp below):
#if defined(BOOST_MSVC) || (defined(__BORLANDC__) && !defined(BOOST_DISABLE_WIN32))
#   define BOOST_TT_DECL __cdecl
#else
#   define BOOST_TT_DECL /**/
#endif

#ifndef BOOST_TT_NO_CONFORMING_IS_CLASS_IMPLEMENTATION
# define BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION 1
#endif

//
// define BOOST_TT_TEST_MS_FUNC_SIGS
// when we want to test __stdcall etc function types with is_function etc
// (Note, does not work with Borland, even though it does support __stdcall etc):
//
#if defined(_MSC_EXTENSIONS) && !defined(__BORLANDC__)
#  define BOOST_TT_TEST_MS_FUNC_SIGS
#endif

//
// Macros that have been deprecated, defined here for backwards compatibility:
//
#define BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(x)
#define BOOST_TT_BROKEN_COMPILER_SPEC(x)

#endif // BOOST_TT_CONFIG_HPP_INCLUDED


