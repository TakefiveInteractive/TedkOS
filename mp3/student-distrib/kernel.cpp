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
#include <inc/mbi_info.h>
#include <inc/klibs/palloc.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/x86/desc.h>
#include <inc/x86/stacker.h>
#include <inc/init.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/x86/real.h>
#include <inc/ui/vbe.h>
#include <inc/klibs/stack.h>

using namespace palloc;
using arch::Stacker;
using arch::CPUArchTypes::x86;
using namespace vbe;

using scheduler::makeKThread;
using scheduler::forceStartThread;

volatile bool canUseCpp = false;

/* Initialize runtime library */
extern "C" void _init(void);

// Make sure usable_mem has at least 12KB memory (later it will be 5MB memory.)
// It uses the two aligned arrays declared below.
void kernel_enable_basic_paging();

uint32_t basicPageDir[1024] __attribute__((aligned (4096)));
uint32_t basicPageTable0[1024] __attribute__((aligned (4096)));
uint32_t userFirst4MBTable[1024] __attribute__((aligned (4096)));

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
    void
_entry (unsigned long magic, unsigned long addr)
{
    /* First thing we do is to assign the global MBI */
    MultiBootInfoAddress = (multiboot_info_t *) addr;

    /* Clear the screen. */
    clear();

    printf ("mbi = 0x%#x\n", (uint32_t) addr);

    /* Print boot info */
    //mbi_info(magic, addr);

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

        // should load kernel ds,es,fs,gs so that at interrupt we have correct data access
        tss.ds = tss.es = tss.fs = tss.gs = KERNEL_DS_SEL;
        ltr(KERNEL_TSS_SEL);
    }

    /* Paging */
    kernel_enable_basic_paging();

    /* Init the PIC */
    i8259_init();

    /* Init the interrupts */
    init_idt();

    /* Initialize runtime library */
    _init();
    canUseCpp = true;

    /* initialize scheduler variables */
    scheduler::init();

    /* Initialize file system */
    filesystem::Dispatcher::init();

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */

    for(size_t i = 0; i < num_known_drivers; i++)
    {
        printf("Loading driver '%s'...", known_drivers[i].name);
        known_drivers[i].init();
        printf("OK!\n");
    }

    // Call this only after C++ initialization has been completed.
    prepareRealMode();

    // ----- START init as a KERNEL thread (because its code is in kernel code) -----
    forceStartThread(makeKThread(init_main));

    // This part should never be reached.
    printf("Halted.\n");
    asm volatile("1: hlt; jmp 1b;");

}

void kernel_enable_basic_paging()
{
    AutoSpinLock lock(&cpu0_paging_lock);
    int32_t i;
    uint32_t* pageDir   = basicPageDir;
    uint32_t* pageTable = basicPageTable0;
    memset(pageDir  , 0, 0x1000);
    memset(pageTable, 0, 0x1000);
    REDIRECT_PAGE_DIR(pageDir);
    LOAD_4MB_PAGE(1, 1 << 22, PG_WRITABLE);
    LOAD_PAGE_TABLE(0, pageTable, PT_WRITABLE);

    // IMPORTANT!!! Must start from i = 1. NOT i = 0 !!!!!
    for(i = 1; i < 0x400; i++)
    {
        LOAD_4KB_PAGE(0, i, i << 12, PG_WRITABLE);
    }

    // ------- initialize userFirst4MBTable --------
    uint32_t id = PRE_INIT_VIDEO >> 12; //id: page id for vmem
    memset(userFirst4MBTable, 0, 0x1000);
    userFirst4MBTable[id] = (uint32_t)PG_4KB_BASE | (id << 12) | PG_WRITABLE | PG_USER;

    enable_paging();
}

extern "C" void __attribute__((externally_visible))
entry (unsigned long magic, unsigned long addr)
{
    _entry(magic, addr);
}
