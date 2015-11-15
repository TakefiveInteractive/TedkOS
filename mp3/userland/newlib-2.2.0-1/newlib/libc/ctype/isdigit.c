/*
FUNCTION
<<isdigit>>---decimal digit predicate

INDEX
isdigit

ANSI_SYNOPSIS
#include <ctype.h>
int isdigit(int <[c]>);

TRAD_SYNOPSIS
#include <ctype.h>
int isdigit(<[c]>);

DESCRIPTION
<<isdigit>> is a macro which classifies ASCII integer values by table
lookup.  It is a predicate returning non-zero for decimal digits, and 0 for
other characters.  It is defined only if <[c]> is representable as an
unsigned char or if <[c]> is EOF.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isdigit>>'.

RETURNS
<<isdigit>> returns non-zero if <[c]> is a decimal digit (<<0>>--<<9>>).

PORTABILITY
<<isdigit>> is ANSI C.

No supporting OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>


#undef isdigit
int
_DEFUN(isdigit,(c),int c)
{
	return(__ctype_ptr__[c+1] & _N);
}
