# /x86/desc.S - Set up x86 segment descriptors, descriptor tables 
# vim:ts=4 noexpandtab

#define ASM     1
#include <inc/x86/desc.h>
#include <inc/x86/desc_interrupts.h>

.text

.globl  ldt_size, tss_size
.globl  ldt_selector, tss_selector
.globl  tss, tss_desc, ldt, ldt_desc
.globl  gdt_ptr
.globl  gdtr_val, idtr_val, idt

.align 4


tss_size:
	.long tss_bottom - tss - 1

ldt_size:
	.long ldt_bottom - ldt - 1

	.word 0 # Padding
ldt_selector:
	.word KERNEL_LDT_SEL
	.long ldt		# This long number is NEVER used.

	.align 4

# ************* GDT, IDT, TSS, and LDT TABLEs **************

	.align  16
gdt:
_gdt:

	# First GDT entry cannot be used
	.quad 0
	
	# NULL entry
	.quad 0
	
	# Segmentation will not be used
	# CS and DS both are 0-4GB r/w segments
	#
	# The layout is (from Intel IA-32 reference manual):
	#  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
	# |----------------------------------------------------------------------|
	# |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
	# | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
	# |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
	# |----------------------------------------------------------------------|
	#
	# |----------------------------------------------------------------------|
	# |                                    |                                 |
	# | Base 15:0                          | Segment Limit 15:0              |
	# |                                    |                                 |
	# |----------------------------------------------------------------------|
	
gdt_ptr:
	# Set up a descriptor for kernel CS
	.quad 0x00CF9A000000FFFF

	# Set up a descriptor for kernel DS
	.quad 0x00CF92000000FFFF

	# Set up a descriptor for user CS
	.quad 0x00CFFA000000FFFF

	# Set up a descriptor for user DS
	.quad 0x00CFF2000000FFFF

	# Set up a descriptor for TSS
tss_desc:
	.quad 0

	# Set up a descriptor LDT
ldt_desc:
	.quad 0

gdt_bottom:
# This is end of GDT table


# [LOCAL]     This is LDT **TABLE**, NOT the descriptor in gdt table for ldt.
	.align 16
ldt:
	.rept 4
	.quad 0
	.endr
ldt_bottom:
# This is end of LDT TABLE


# [INTERRUPT] This is IDT Table
	.align  16
idt:
_idt:
	.rept NUM_VEC
	.quad 0
	.endr

idt_bottom:
# This is end of IDT Table

# [TASK SWTICH] This is TSS **TABLE**, NOT the descriptor in gdt table for tss.
tss:
_tss:
	.rept 104
	.byte 0
	.endr
tss_bottom:
# End of TSS TABLE

# ************* IDTR and GDTR's values **************
# This is the value to be loaded into idtr
.align 4
	.word 0 # Padding
idtr_val:
	.word idt_bottom - idt - 1
	.long idt
	
# This is the value to be loaded into gdtr
.align 4
	.word 0 # Padding
gdtr_val:
	.word gdt_bottom - gdt - 1
	.long gdt
	
