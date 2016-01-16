.data

.globl ascii_shift_table
.align 4

# this is a 128 byte array
.align 4
ascii_shift_table:
    # These bytes are not valid
    .rept 39
    .byte 0
    .endr

    .byte '"'

    # These bytes are not valid
    .rept 4
    .byte 0
    .endr

    .byte '<'
    .byte '_'
    .byte '>'
    .byte '?'

    # SHIFT of 0 - 9
    .byte ')'
    .byte '!'
    .byte '@'
    .byte '#'
    .byte '$'
    .byte '%'
    .byte '^'
    .byte '&'
    .byte '*'
    .byte '('

    # This byte is not valid
    .byte 0

    .byte ':'

    # This byte is not valid
    .byte 0

    .byte '+'

    # These bytes are not valid
    .rept 29
    .byte 0
    .endr

    .byte '{'
    .byte '|'
    .byte '}'

    # These bytes are not valid
    .byte 0
    .byte 0
    
    .byte '~'
    .byte 'A'
    .byte 'B'
    .byte 'C'
    .byte 'D'
    .byte 'E'
    .byte 'F'
    .byte 'G'
    .byte 'H'
    .byte 'I'
    .byte 'J'
    .byte 'K'
    .byte 'L'
    .byte 'M'
    .byte 'N'
    .byte 'O'
    .byte 'P'
    .byte 'Q'
    .byte 'R'
    .byte 'S'
    .byte 'T'
    .byte 'U'
    .byte 'V'
    .byte 'W'
    .byte 'X'
    .byte 'Y'
    .byte 'Z'

    # These bytes are not valid
    .rept 5
    .byte 0
    .endr

