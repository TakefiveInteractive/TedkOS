#ifndef _BOOST_CONFIG_H_
#define _BOOST_CONFIG_H_

#define BOOST_STATIC_CONSTANT(type, assignment) static const type assignment

#if defined(BOOST_NO_CONSTEXPR)
#define BOOST_CONSTEXPR
#define BOOST_CONSTEXPR_OR_CONST const
#else
#define BOOST_CONSTEXPR constexpr
#define BOOST_CONSTEXPR_OR_CONST constexpr
#endif

#define BOOST_STATIC_CONSTEXPR  static BOOST_CONSTEXPR_OR_CONST

#define BOOST_NO_LONG_LONG

// BOOST_NESTED_TEMPLATE workaround ------------------------------------------//
// Member templates are supported by some compilers even though they can't use
// the A::template member<U> syntax, as a workaround replace:
//
// typedef typename A::template rebind<U> binder;
//
// with:
//
// typedef typename A::BOOST_NESTED_TEMPLATE rebind<U> binder;

#ifndef BOOST_NO_MEMBER_TEMPLATE_KEYWORD
#  define BOOST_NESTED_TEMPLATE template
#else
#  define BOOST_NESTED_TEMPLATE
#endif

#ifdef BOOST_NO_CXX11_NOEXCEPT
#  define BOOST_NOEXCEPT
#  define BOOST_NOEXCEPT_IF(Predicate)
#  define BOOST_NOEXCEPT_EXPR(Expression) false
#else
#  define BOOST_NOEXCEPT noexcept
#  define BOOST_NOEXCEPT_IF(Predicate) noexcept((Predicate))
#  define BOOST_NOEXCEPT_EXPR(Expression) noexcept((Expression))
#endif

//
// Helper macro BOOST_JOIN:
// The following piece of macro magic joins the two
// arguments together, even when one of the arguments is
// itself a macro (see 16.3.1 in C++ standard).  The key
// is that macro expansion of macro arguments does not
// occur in BOOST_DO_JOIN2 but does in BOOST_DO_JOIN.
//
#define BOOST_JOIN( X, Y ) BOOST_DO_JOIN( X, Y )
#define BOOST_DO_JOIN( X, Y ) BOOST_DO_JOIN2(X,Y)
#define BOOST_DO_JOIN2( X, Y ) X##Y

#ifndef BOOST_NO_DEDUCED_TYPENAME
#  define BOOST_DEDUCED_TYPENAME typename
#else
#  define BOOST_DEDUCED_TYPENAME
#endif

#include <stdint.h>
namespace std {
    template <class T> void swap ( T& a, T& b )
    {
        T c(a); a=b; b=c;
    }
    typedef ::size_t size_t;
}

#endif

