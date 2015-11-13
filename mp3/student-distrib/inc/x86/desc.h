/* /inc/x86/desc.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 expandtab
 */

#ifndef _X86_DESC_H
#define _X86_DESC_H


// !!! This header is also included in ASM !!!
// !!!  Be careful to add C types and function
//  declarations in #ifndef ASM part

/*
 * Warning!!!!
 *	Remeber: index of the selector = selector's value >> 3 !!!
 */
/* Segment SELECTOR values */
#define KERNEL_CS_SEL 0x0010
#define KERNEL_DS_SEL 0x0018
#define USER_CS_SEL 0x0023
#define USER_DS_SEL 0x002B
#define KERNEL_TSS_SEL 0x0030
#define KERNEL_LDT_SEL 0x0038

/* Size of the task state segment (TSS) */
#define TSS_SIZE 104

#ifndef ASM

#include <inc/x86/desc_interrupts.h>

/* This structure is used to load descriptor base registers
 * like the GDTR and IDTR
 * EXTRA NOTES:
 *	It represents the value stored in GDTR and IDTR
 *	We RENAMED this structure, in order to clarify the meaning.
 *	Its original name was struct x86_desc and x86_desc_t
 * New name: idtr_val_t and gdtr_val_t
 *	This is short for x86_?dt_register_value where ? can either be gdt or idt
 * !!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *      GDTR and LDTR's values are not in the format of SELECTOR,
 *       but IDTR and TR (TSS register)'s values ARE.
 */

typedef struct __attribute__((packed)) {
    int32_t edi;
    int32_t esi;
    uint32_t ebp;
    uint32_t esp;
    int32_t ebx;
    int32_t edx;
    int32_t ecx;
    int32_t eax;
} pushal_t;

typedef struct {
	uint16_t padding;
	uint16_t size;
	uint32_t addr;
} gdtr_val_t;
typedef gdtr_val_t idtr_val_t;

/* This is a segment descriptor.  It goes in the GDT. */
typedef struct seg_desc {
	union {
		uint32_t val;
		struct {
			uint16_t seg_lim_15_00;
			uint16_t base_15_00;
			uint8_t base_23_16;
			uint32_t type : 4;
			uint32_t sys : 1;
			uint32_t dpl : 2;
			uint32_t present : 1;
			uint32_t seg_lim_19_16 : 4;
			uint32_t avail : 1;
			uint32_t reserved : 1;
			uint32_t opsize : 1;
			uint32_t granularity : 1;
			uint8_t base_31_24;
		} __attribute__((packed));
	};
} seg_desc_t;

/* TSS structure */
typedef struct __attribute__((packed)) tss_t {
	uint16_t prev_task_link;
	uint16_t prev_task_link_pad;

	uint32_t esp0;
	uint16_t ss0;
	uint16_t ss0_pad;

	uint32_t esp1;
	uint16_t ss1;
	uint16_t ss1_pad;

	uint32_t esp2;
	uint16_t ss2;
	uint16_t ss2_pad;

	uint32_t cr3;

	uint32_t eip;
	uint32_t eflags;

	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;

	uint16_t es;
	uint16_t es_pad;

	uint16_t cs;
	uint16_t cs_pad;

	uint16_t ss;
	uint16_t ss_pad;

	uint16_t ds;
	uint16_t ds_pad;

	uint16_t fs;
	uint16_t fs_pad;

	uint16_t gs;
	uint16_t gs_pad;

	uint16_t ldt_segment_selector;
	uint16_t ldt_pad;

	uint16_t debug_trap : 1;
	uint16_t io_pad : 15;
	uint16_t io_base_addr;
} tss_t;

/* Some external descriptors declared in .S files */

/* the value to be loaded into gdtr */
extern gdtr_val_t gdtr_val;

extern uint16_t ldt_selector;
extern uint32_t ldt_size;
extern seg_desc_t ldt_desc;
extern seg_desc_t gdt_ptr;
extern uint32_t ldt;

extern uint32_t tss_size;
extern seg_desc_t tss_desc;
extern tss_t tss;

/* Sets runtime-settable parameters in the GDT entry for the LDT */
#define SET_LDT_PARAMS(str, addr, lim) \
do { \
	str.base_31_24 = ((uint32_t)(addr) & 0xFF000000) >> 24; \
		str.base_23_16 = ((uint32_t)(addr) & 0x00FF0000) >> 16; \
		str.base_15_00 = (uint32_t)(addr) & 0x0000FFFF; \
		str.seg_lim_19_16 = ((lim) & 0x000F0000) >> 16; \
		str.seg_lim_15_00 = (lim) & 0x0000FFFF; \
} while(0)

/* Sets runtime parameters for the TSS */
#define SET_TSS_PARAMS(str, addr, lim) \
do { \
	str.base_31_24 = ((uint32_t)(addr) & 0xFF000000) >> 24; \
		str.base_23_16 = ((uint32_t)(addr) & 0x00FF0000) >> 16; \
		str.base_15_00 = (uint32_t)(addr) & 0x0000FFFF; \
		str.seg_lim_19_16 = ((lim) & 0x000F0000) >> 16; \
		str.seg_lim_15_00 = (lim) & 0x0000FFFF; \
} while(0)

/* An interrupt descriptor entry (goes into the IDT) */
// This is ONLY TRAP Gate and INTERRUPT Gate
typedef union idt_desc_t {
	uint32_t val;                    // ACTUALLY this UNION is 64-bits long !
	struct {
		uint16_t offset_15_00;
		uint16_t seg_selector;
		uint8_t reserved4;           // This value MUST be initialized to 0
		uint32_t reserved3 : 1;      // Interrupt Gate=0. Trap Gate=1
		uint32_t reserved2 : 1;      // MUST be initialized to 1
		uint32_t reserved1 : 1;      // MUST be initialized to 1
		uint32_t size : 1;           // Corresponds to "D" on IA-32 manual.
		uint32_t reserved0 : 1;      // MUST be initialized to 0
		uint32_t dpl : 2;
		uint32_t present : 1;
		uint16_t offset_31_16;
	} __attribute__((packed));
} idt_desc_t;

/* The IDT itself (declared in /x86/desc.S */
extern idt_desc_t idt[NUM_VEC];
/* The value to be loaded into the IDTR */
/* This variabl was named idt_desc_ptr and renamed to avoid confusion */
extern idtr_val_t idtr_val;

/*
 * IMPORTANT!!!! Difference between INTERRUPT Gate and TRAP Gate:
 *    Interrupt Gate will DISABLE interrupt before executing handler.
 *    Trap Gate will not DISABLE interrupt.
 *    (In other words, Interrupt Gate changes IF flags, but Trap Gate does not)
 */

// Initialize a TRAP gate as IDT descriptor
// PARAMS: desc -- stores the IDT desc here. TYPE: idt_desc_t
//         base -- the selector for base segment containing the code
//           |_,-- TYPE (of base): uint16_t, selector.
// WARNING: Default SIZE of gate to be 32-bit (not 16-bit)
// WARNING: This will NOT set DPL,Present, OR Offset
#define INIT_TRAP_DESC(desc, base)  {      \
    (desc).seg_selector = (base);          \
    (desc).size         = 1;               \
    (desc).reserved4    = 0;               \
    (desc).reserved3    = 1;               \
    (desc).reserved2    = 1;               \
    (desc).reserved1    = 1;               \
    (desc).reserved0    = 0;               \
}

// Initialize a INTERRUPT gate as IDT descriptor
// PARAMS: desc -- stores the IDT desc here. TYPE: idt_desc_t
//         base -- the selector for base segment containing the code
//           |_,-- TYPE (of base): uint16_t, selector.
// WARNING: Default SIZE of gate to be 32-bit (not 16-bit)
// WARNING: This will NOT set DPL,Present, OR Offset
#define INIT_INT_DESC(desc, base)  {       \
    (desc).seg_selector = (base);          \
    (desc).size         = 1;               \
    (desc).reserved4    = 0;               \
    (desc).reserved3    = 0;               \
    (desc).reserved2    = 1;               \
    (desc).reserved1    = 1;               \
    (desc).reserved0    = 0;               \
}

/* Sets runtime parameters for an IDT entry */
#define SET_IDT_DESC_OFFSET(desc, handler) \
do { \
	(desc).offset_31_16 = ((uint32_t)(handler) & 0xFFFF0000) >> 16; \
		(desc).offset_15_00 = ((uint32_t)(handler) & 0xFFFF); \
} while(0)

/* Load task register.  This macro takes a 16-bit index into the GDT,
 * which points to the TSS entry.  x86 then reads the GDT's TSS
 * descriptor and loads the base address specified in that descriptor
 * into the task register */
#define ltr(desc)                       \
do {                                    \
	asm volatile("ltr %w0"              \
			:                           \
			: "r" (desc)                \
			: "memory", "cc" );         \
} while(0)

/* Load the interrupt descriptor table (IDT).  This macro takes a 32-bit
 * address which points to a 6-byte structure.  The 6-byte structure
 * (defined as "struct x86_desc" above) contains a 2-byte size field
 * specifying the size of the IDT, and a 4-byte address field specifying
 * the base address of the IDT. */
#define lidt(desc)                      \
do {                                    \
	asm volatile("lidt (%0)"            \
			:                           \
			: "g" (desc)                \
			: "memory");                \
} while(0)

/* Load the local descriptor table (LDT) register.  This macro takes a
 * 16-bit index into the GDT, which points to the LDT entry.  x86 then
 * reads the GDT's LDT descriptor and loads the base address specified
 * in that descriptor into the LDT register */
#define lldt(desc)                      \
do {                                    \
	asm volatile("lldt %%ax"            \
			:                           \
			: "a" (desc)                \
			: "memory" );               \
} while(0)

#endif /* ASM */

#endif /* _x86_DESC_H */
