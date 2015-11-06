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
#include <inc/tests.h>
#include <inc/fs/kiss_wrapper.h>
#include <inc/mbi_info.h>

/* Initialize runtime library */
extern "C" void _init(void);

// Make sure usable_mem has at least 12KB memory (later it will be 5MB memory.)
// It uses the two aligned arrays declared below.
void kernel_enable_basic_paging();

uint32_t basicPageDir[1024] __attribute__((aligned (4096)));
uint32_t basicPageTable0[1024] __attribute__((aligned (4096)));

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
    void
_entry (unsigned long magic, unsigned long addr)
{
    /* First thing we do is to assign the global MBI */
    MultiBootInfoAddress = reinterpret_cast<multiboot_info_t *>(addr);

    /* Clear the screen. */
    clear();

    /* Print boot info */
    mbi_info(magic, addr);

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
    kernel_enable_basic_paging();

    /* Init the PIC */
    i8259_init();

    /* Init the interuupts */
    init_idt();

    /* Initialize runtime library */
    _init();

    /* Initialize file system */
    filesystem::dispatcher.mountAll();

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */
    for(size_t i = 0; i < num_known_drivers; i++)
    {
        printf("Loading driver '%s' ...", known_drivers[i].name);
        known_drivers[i].init();
        printf(" ... OK!\n");
    }

    /* Enable interrupts */
    sti();

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

    // Test system call
    __asm__ __volatile__("push %%eax; push %%ebx; mov $3, %%eax; mov $5, %%ebx; int $0x80; pop %%ebx; pop %%eax; \n" : :);

    //while(1)
    {
        int rval;
        printf("trying to start test programs... ");
        rval = kshell_main();
        if(rval)
            printf("FAILED.\n");
    }

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile(".1: hlt; jmp .1;");
}

void kernel_enable_basic_paging()
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

