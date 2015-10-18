
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

call_table:
        .long   mp1_ioctl_add
        .long   mp1_ioctl_remove
        .long   mp1_ioctl_find
        .long   mp1_ioctl_sync

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
        
        movl    vmem_base_addr(,1),%edx
        movb    %cl,(%edx,%eax,1)
        ret

mp1_rtc_tasklet:

        pushl   %ebp
        movl    %esp, %ebp

        pushl   %esi
        pushl   %edi
	pushl	%ebx

        movl    mp1_list_head, %ebx
        
tasklet_loop:
        cmpl    $0, %ebx
        je      tasklet_end

        decw    COUNTDOWN(%ebx)
        cmpw    $0,COUNTDOWN(%ebx)
        jg      next_loop

        movzwl  LOCATION(%ebx), %eax  #blink location is now in eax
        shl     $1,%eax

        testw   $0x1,STATUS(%ebx)
        jz      currently_off

        movb    OFF_CHAR(%ebx), %cl
        call    mp1_poke
        movw    OFF_LENGTH(%ebx),%dx
        jmp     end_blink

currently_off:
        movb    ON_CHAR(%ebx), %cl
        call    mp1_poke
        movw    ON_LENGTH(%ebx),%dx

end_blink:
        movw    %dx, COUNTDOWN(%ebx)
        xorw    $0x1, STATUS(%ebx)

next_loop:
        movl    NEXT(%ebx), %ebx
        jmp     tasklet_loop

tasklet_end:

	popl	%ebx
        popl    %edi
        popl    %esi

        popl    %ebp
        ret

mp1_ioctl:
        movl    8(%esp), %eax
        jmp     *call_table(,%eax,4)

mp1_ioctl_add:
        pushl   %ebp
        movl    %esp, %ebp

        # Allocate temp structure on the stack
        subl    $STRUCT_SIZE, %esp

        # Preserve registers
        pushl   %esi
        pushl   %edi
        pushl   %ebx

        # Get pointer to temp structure
        leal    -STRUCT_SIZE(%ebp), %ebx

        # Copy user data onto the stack
        pushl   $STRUCT_SIZE
        pushl   8(%ebp)
        pushl   %ebx
        call    ece391_memcpy
        addl    $12,%esp

        # Check that the copy succeeded
        cmpl    $0,%eax
        jne     add_fail_return

        # Check that the location is valid
        cmpw    $80*25,LOCATION(%ebx)
        jae     add_fail_return
       
        # Mark this structure as valid
        movw    $0x1,STATUS(%ebx)  # Mark it as on
        movw    ON_LENGTH(%ebx),%dx
        movw    %dx,COUNTDOWN(%ebx)

        # Allocate some memory, pointer returned in EAX
        pushl   $STRUCT_SIZE
        call    mp1_malloc
        add     $4, %esp
	cmpl	$0, %eax
	je	add_fail_return

        # Save EAX
        pushl   %eax

        pushl   $STRUCT_SIZE
        pushl   %ebx
        pushl   %eax
        call    ece391_memcpy
        addl    $12, %esp

        # Restore the value from EAX into EDX
        popl    %edx

        # Insert this new item at the front of the list
        movl    mp1_list_head, %eax
        movl    %eax, NEXT(%edx)
        movl    %edx, mp1_list_head

display:
        # Display the character
        movzwl  LOCATION(%edx),%eax
        shll    $1,%eax
        movb    ON_CHAR(%edx),%cl
        call    mp1_poke
        jmp     add_success_return

add_fail_return:
        movl    $-1,%eax
        jmp     add_leave

add_success_return:
        movl    $0, %eax
add_leave:
        popl    %ebx
        popl    %edi
        popl    %esi

        leave
        ret
        
        
mp1_ioctl_remove:
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %esi
        pushl   %edi
        pushl   %ebx

        pushl   8(%ebp)
        call    mp1_find_helper
        addl    $4, %esp
        cmpl    $0, %eax
        je      remove_fail_return
    
        # Found the right element.  Now find the
        # previous pointer in the list, and remove it
        # We should not fail here, because mp1_find_helper
        # returned a valid list element

        leal    mp1_list_head, %edx
remove_find_prev:
        cmp     %eax, (%edx)
        je      found_prev
        movl    (%edx), %edx
        leal    NEXT(%edx), %edx
        jmp     remove_find_prev

found_prev:
        # Perform the removal
        movl    NEXT(%eax), %ecx
        movl    %ecx, (%edx)

free_mem:
        pushl   %eax
        call    mp1_free
        addl    $4, %esp
        jmp     remove_success_return

remove_fail_return:
        movl    $-1,%eax
        jmp     remove_leave

remove_success_return:
        movl    $0, %eax
remove_leave:
        popl    %ebx
        popl    %edi
        popl    %esi

        leave
        ret
        
 
mp1_ioctl_sync:
        pushl   %ebp
        movl    %esp, %ebp

        pushl   %esi
        pushl   %edi
        pushl   %ebx

sync_find_first:
        movl    8(%ebp), %eax
        shr     $16, %eax
        pushl   %eax
        call    mp1_find_helper
        addl    $4, %esp
        cmpl    $0, %eax
        je      sync_fail_return
        movl    %eax, %esi

        movl    8(%ebp), %eax
        andl    $0xFFFF, %eax
        pushl   %eax
        call    mp1_find_helper
        addl    $4, %esp
        cmpl    $0, %eax
        je      sync_fail_return
        movl    %eax, %edi

sync_copy_loop:
        movw    ON_LENGTH(%esi), %ax
        movw    %ax, ON_LENGTH(%edi)
        movw    OFF_LENGTH(%esi), %ax
        movw    %ax, OFF_LENGTH(%edi)
        movw    COUNTDOWN(%esi), %ax
        movw    %ax, COUNTDOWN(%edi)
        movw    STATUS(%esi), %ax
        movw    %ax, STATUS(%edi)

        movzwl  LOCATION(%edi), %eax
        shll    $1,%eax
        movzbl  OFF_CHAR(%edi),%ecx
        movzbl  ON_CHAR(%edi),%ebx
        testb   $0x1,STATUS(%edi)
        cmovl   %ebx, %ecx

sync_display:
        call    mp1_poke
        jmp     sync_success_return

sync_fail_return:
        movl    $-1,%eax
        jmp     sync_leave

sync_success_return:
        movl    $0, %eax
sync_leave:
        popl    %ebx
        popl    %edi
        popl    %esi

        leave
        ret

mp1_ioctl_find:
        pushl   %ebp
        movl    %esp, %ebp
        subl    $2,%esp

        pushl   %esi
        pushl   %edi
        pushl   %ebx

        pushl   $2
        pushl   8(%ebp)
        leal    -2(%ebp),%edi
        pushl   %edi
        call    ece391_memcpy
        addl    $12,%esp

        cmp     $0,%eax
        jne     find_fail_return

        movzwl  -2(%ebp),%eax
        pushl   %eax
        call    mp1_find_helper
        addl    $4, %esp
        cmpl    $0, %eax
        je      find_fail_return

        pushl   $STRUCT_SIZE
        pushl   %eax
        pushl   8(%ebp)
        call    ece391_memcpy
        addl    $12,%esp

        cmp     $0,%eax
        je      find_success_return

find_fail_return:
        movl    $-1,%eax
        jmp     find_leave

find_success_return:
        movl    $0, %eax
find_leave:
        popl    %ebx
        popl    %edi
        popl    %esi

        leave
        ret

mp1_find_helper:
        pushl	%ebp
        movl	%esp, %ebp

        pushl   %ebx

        movw	8(%ebp), %ax
        movl	mp1_list_head, %ebx
loop:
        cmpl    $0, %ebx
        je      helper_fail_return

        cmpw    %ax, LOCATION(%ebx)
        je      helper_success_return

        movl    NEXT(%ebx), %ebx
        jmp     loop

helper_fail_return:
        xorl    %eax, %eax
        jmp     helper_leave

helper_success_return:
        movl    %ebx, %eax

helper_leave:
        popl    %ebx
        leave
        ret

.end
