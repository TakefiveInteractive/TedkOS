#include "ata_priv.h"
#include <inc/drivers/ata.h>
#include <inc/drivers/pci.h>
#include <inc/drivers/scan_pci.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/maybe.h>
#include <inc/klibs/function.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>

using namespace pci;

/*
 * This code is copied from https://github.com/mallardtheduck/osdev and dramatically revised.
 */

namespace ata {

static bool dma_init=false;

const int DMA_ON = 1;
const int DMA_READ = 8;

const int bus0_io_base=0x1F0;
const int bus1_io_base=0x170;

function<void ()>* dma_finish_read[2];
bool dma_waiting_read[2] = {false, false};

struct prd{
    uint32_t data;
    uint16_t bytes;
    uint16_t reserved;

    prd() : reserved((1 << 15)) {}
} __attribute__((packed));

prd bus0prd __attribute__((aligned(8))), bus1prd __attribute__((aligned(8)));
spinlock_t dma_lock, dma_init_lock;
uint32_t bmr;

int dma_int_handler(int irq, unsigned int saved_reg){
    dbgpf("ATA DMA: interrupt!\n");

    uint8_t bus0status=inb(bmr+0x02);
    uint8_t bus1status=inb(bmr+0x0A);
    if(bus0status & 0x04){
        dbgpf("ATA DMA: Bus 0 interrupt!\n");
        dbgpf("ATA DMA: Bus 0 status: %x\n", bus0status);
        osdev_outb(bmr+0x02, 0x04);

        if(dma_waiting_read[0])
            (*dma_finish_read[0])();
    }
    if(bus1status & 0x04){
        dbgpf("ATA DMA: Bus 1 interrupt!\n");
        dbgpf("ATA DMA: Bus 1 status: %x\n", bus1status);
        osdev_outb(bmr+0x0A, 0x04);

        if(dma_waiting_read[1])
            (*dma_finish_read[1])();
    }
    return 0;
}

inline uint32_t physaddr(void* ptr) {
    return (cpu0_memmap.translate(palloc::VirtAddr(ptr)).pde & ALIGN_4MB_ADDR) | (((uint32_t)ptr) & (~ALIGN_4MB_ADDR));
}

void set_bus0_prdt(uint32_t bmr, prd *prd){
    uint32_t phys=physaddr(prd);
    dbgpf("ATA DMA: Setting bus 0 PRDT pointer to %x.\n", phys);
    osdev_outl(bmr+0x04, phys);
    bus0prd=*prd;
}

void set_bus1_prdt(uint32_t bmr, prd *prd){
    uint32_t phys=physaddr(prd);
    dbgpf("ATA DMA: Setting bus 1 PRDT pointer to %x.\n", phys);
    osdev_outl(bmr+0x0C, phys);
    bus1prd=*prd;
}

bool init_dma(){
    // return false;
    AutoSpinLock hl(&dma_init_lock);
    if(dma_init) return true;
    //if(init_pci()){
        if(!PATAControllers)
            return false;
        if(PATAControllers->empty())
            return false;
        UnitDev dev=(*PATAControllers)[0];
        bmr=Register(dev, 0x20).get();
        bmr&= (~1);

        // RegisterId should be 0x04 according to OSDev
        uint32_t cmdAndStatus = Register(dev, 0x04).get();
        uint16_t cmd=cmdAndStatus & 0xffff;
        uint16_t status=cmdAndStatus >> 16;
        uint16_t newcmd = cmd | 0x07;
        newcmd &= ~(1 << 10);
        Register(dev, 0x04).set((status << 16) | newcmd);

        cmdAndStatus = Register(dev, 0x04).get();
        cmd=cmdAndStatus & 0xffff;
        dbgpf("ATA DMA: STATUS: %x CMD: %x NEWCMD:%x\n", status, cmd, newcmd);

        spin_lock_init(&dma_lock);
        set_bus0_prdt(bmr, new prd());
        set_bus1_prdt(bmr, new prd());

        bind_irq(14, ATA_DMA_ID, dma_int_handler, 0);
        bind_irq(15, ATA_DMA_ID, dma_int_handler, 0);
        dma_init=true;
        return true;
    //}
    //return false;
}

int32_t dma_begin_read_sector(ata_device *dev, uint32_t lba, uint8_t *buf, uint32_t nbytes){

    if(!dma_init) return -EFOPS;
    if(nbytes % ATA_SECTOR_SIZE)
        return -EFOPS;

    size_t bufferSize = (nbytes % 0x1000) ? ((nbytes / 0x1000) + 1) * 0x1000 : nbytes;
    auto dma_buffer = new char [bufferSize];

    dbgpf("ATA DMA: DMA read: dev: %x, lba: %x, buf: %x\n", dev, lba, physaddr((void*)buf));
    memset(buf, 0, nbytes);
    int bus=dev->io_base;
    int slave=dev->slave;

    uint16_t base;
    size_t busId;

    AutoSpinLock hl(&dma_lock);
    if(dev->io_base==bus0_io_base){
        bus0prd.data=physaddr((void*)dma_buffer);
        bus0prd.bytes=nbytes;
        busId = 0;
        base=0;
        set_bus0_prdt(bmr, &bus0prd);
        dbgout("ATA DMA: Primary.\n");
    }else if(dev->io_base==bus1_io_base){
        bus1prd.data=physaddr((void*)dma_buffer);
        bus1prd.bytes=nbytes;
        busId = 1;
        base=0x08;
        set_bus1_prdt(bmr, &bus1prd);
        dbgout("ATA DMA: Secondary.\n");
    }else{
        panic("(ATA DMA) Unrecognised device!");
        return -EFOPS;
    }
    osdev_outb(bus + ATA_REG_CONTROL, 2);
    osdev_outb(bmr + base, DMA_READ);

    uint8_t status=inb(bmr + base + 2);
    osdev_outb(bmr + base + 2, status | 2 | 4);

    ata_wait(dev, 0);
    osdev_outb(bus + ATA_REG_CONTROL, 0);

    osdev_outb(bus + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
    osdev_outb(bus + 1, 0x03);
    osdev_outb(bus + 2, 0x21);
    osdev_outb(bus + ATA_REG_COMMAND, 0xEF);
    uint8_t q=inb(ATA_REG_STATUS);
    dbgpf("ATA DMA: q=%x\n", q);
    //osdev_outb(bus + ATA_REG_FEATURES, 0x01);
    osdev_outb(bus + ATA_REG_SECCOUNT0, 1);
    osdev_outb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
    osdev_outb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
    osdev_outb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
    osdev_outb(bus + ATA_REG_COMMAND, ATA_CMD_READ_DMA);

    uint8_t cmd=inb(bmr + base);
    osdev_outb(bmr + base, cmd | DMA_ON);

    dbgpf("ATA DMA: Waiting for DMA to complete...\n");

    thread_kinfo* thisThread = getCurrentThreadInfo();

    dma_waiting_read[busId] = true;
    dma_finish_read[busId] = new function<void ()> ([=]()
    {
        osdev_outb(bmr + base, 0);
        uint32_t retval;
        ata_wait(dev, 1);
        uint8_t atastatus=0;
        if(inb(bmr + base+2) & 0x02) atastatus=1;
        if(atastatus){
            panic("(ATA DMA) DMA FAILED");
            dbgpf("ATA DMA: DMA error!\n");
            // TODO: FIXME: should do a SOFTWARE RESET before return.
            retval = -EFOPS;
        }else{
            cpu0_memmap.loadProcessMap(thisThread->getProcessDesc());
            memcpy(buf, dma_buffer, nbytes);
            cpu0_memmap.loadProcessMap(getCurrentThreadInfo()->getProcessDesc());
            dbgpf("ATA DMA: DMA complete.\n");
            retval = nbytes;
        }

        getRegs(thisThread)->eax = retval;
        scheduler::unblock(thisThread);
        dma_waiting_read[busId] = false;
        delete[] dma_buffer;
    });

    scheduler::block(thisThread);
    return -EFOPS;  // This return value is not received by any thread.
}

void preinit_dma(){
    spin_lock_init(&dma_init_lock);
    dma_waiting_read[0] = false;
    dma_waiting_read[1] = false;
}

}   // namespace ata
