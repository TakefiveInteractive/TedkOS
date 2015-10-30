/* kernel.c - the C part of the kernel
 * vim:ts=4 expandtab
 */
#include <stddef.h>
#include <stdint.h>

#include <inc/multiboot.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/desc.h>
#include <inc/klibs/lib.h>
#include <inc/x86/idt_init.h>
#include <inc/i8259.h>
#include <inc/debug.h>
#include <inc/known_drivers.h>
#include <inc/x86/paging.h>
#include <inc/x86/filesystem_wrapper.h>

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// Make sure usable_mem has at least 12KB memory (later it will be 5MB memory.)
// It uses the two aligned arrays declared below.
void kernel_enable_basic_paging(void* usable_mem);

uint32_t basicPageDir[1024] __attribute__((aligned (4096)));
uint32_t basicPageTable0[1024] __attribute__((aligned (4096)));

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
_entry (unsigned long magic, unsigned long addr)
{
    int i;
	multiboot_info_t *mbi;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		size_t mod_count = 0;
		module_t* mod = (module_t*) mbi->mods_addr;
		while (mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int) mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int) mod->mod_end);
			printf("First few bytes of module:\n");
			for (size_t i = 0; i < 16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start + i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc = the_ldt_desc;
		lldt(KERNEL_LDT_SEL);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT_SEL;
		tss.ss0 = KERNEL_DS_SEL;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS_SEL);
	}

    /* Paging */
    kernel_enable_basic_paging(NULL);

	/* Init the PIC */
	i8259_init();

	/* Init the interuupts */
	init_idt();

    /* Initialize file system */
    filesystem::init_from_multiboot(mbi);

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */
    for(i = 0; i < num_known_drivers; i++)
    {
        printf("Loading driver '%s' ...", known_drivers[i].name);
        known_drivers[i].init();
        printf(" ... OK!\n");
    }

	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */
	sti();

	asm volatile("int $0x80;");
    /*
	asm volatile("int $0x22;");
	printf("Let's trigger exception\n");
	i = 1;
	i--;
	i /= i;
    */

	/* Execute the first program (`shell') ... */
    dentry_t dentry;
    read_dentry_by_index(0, &dentry);
    printf("First file: %s\n", dentry.filename);

    read_dentry_by_name((const uint8_t *)"frame0.txt", &dentry);
    uint8_t buf[200] = {};
    size_t len = read_data(dentry.inode, 0, buf, sizeof(buf));
    printf("Loading frame0.txt, size = %d\n", len);
    puts((const char *)buf);

    read_dentry_by_name((const uint8_t *)"frame1.txt", &dentry);
    uint8_t buf1[200] = {};
    size_t len1 = read_data(dentry.inode, 0, buf1, sizeof(buf1));
    printf("Loading frame1.txt, size = %d\n", len1);
    puts((const char *)buf1);

	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}

void kernel_enable_basic_paging(void* usable_mem)
{
    int32_t i;
    uint32_t* pageDir   = basicPageDir;
    uint32_t* pageTable = basicPageTable0;
    memset(pageDir  , 0, 0x1000);
    memset(pageTable, 0, 0x1000);
    REDIRECT_PAGE_DIR(pageDir, 0);
    LOAD_4MB_PAGE(1, 1 << 22, PG_WRITABLE);
    LOAD_PAGE_TABLE(0, pageTable, PT_WRITABLE);

	// IMPORTANT!!! Must start from i = 1. NOT i = 0 !!!!!
    for(i = 1; i < 0x400; i++)
    {
        LOAD_4KB_PAGE(0, i, i << 12, PG_WRITABLE);
    }
    enable_paging();
}

extern "C" void
entry (unsigned long magic, unsigned long addr)
{
    _entry(magic, addr);
}

