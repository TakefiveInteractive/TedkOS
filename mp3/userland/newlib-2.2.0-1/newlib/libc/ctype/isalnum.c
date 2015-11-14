/*
FUNCTION 
	<<isalnum>>---alphanumeric character predicate

INDEX
	isalnum

ANSI_SYNOPSIS
	#include <ctype.h>
	int isalnum(int <[c]>);

TRAD_SYNOPSIS
	#include <ctype.h>
	int isalnum(<[c]>);


DESCRIPTION
<<isalnum>> is a macro which classifies ASCII integer values by table
lookup.  It is a predicate returning non-zero for alphabetic or
numeric ASCII characters, and <<0>> for other arguments.  It is defined
only if <[c]> is representable as an unsigned char or if <[c]> is EOF.

You can use a compiled subroutine instead of the macro definition by
undefining the macro using `<<#undef isalnum>>'.

RETURNS
<<isalnum>> returns non-zero if <[c]> is a letter (<<a>>--<<z>> or
<<A>>--<<Z>>) or a digit (<<0>>--<<9>>).

PORTABILITY
<<isalnum>> is ANSI C.

No OS subroutines are required.
*/

#include <_ansi.h>
#include <ctype.h>

#undef isalnum

int
_DEFUN(isalnum,(c),int c)
{
	return(__ctype_ptr__[c+1] & (_U|_L|_N));
}

