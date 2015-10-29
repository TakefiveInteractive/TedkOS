.data

.globl sp_kkc_handler_table
.globl ascii_shift_table
.align 4

# an array of NUM_SPECIAL_KKC=255 function pointers.
sp_kkc_handler_table:
    .long term_enter_handler
    .long term_backspace_handler
    .long term_delete_handler

    # UP DOWN LEFT RIGHT are not used yet
    .rept 4
    .long term_none_handler
    .endr

    .long term_tab_handler
    .long term_capslock_handler

    # fill up the rest of the table with none_handler
    .rept 255 - 3 - 4 - 2
    .long term_none_handler
    .endr


# this is a 128 byte array
.align 4
ascii_shift_table:
    .rept 44
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

