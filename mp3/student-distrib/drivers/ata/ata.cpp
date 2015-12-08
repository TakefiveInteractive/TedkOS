#include "ata_priv.h"
#include <inc/drivers/ata.h>
#include <inc/drivers/pci.h>
#include <inc/drivers/scan_pci.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/maybe.h>
#include <inc/klibs/function.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/fs/dev_wrapper.h>

#include <inc/klibs/lib.h>

/*
 * This code is copied from https://github.com/mallardtheduck/osdev and slightly revised.
 */

namespace ata {

char dbgbuf[256];

spinlock_t ata_lock, ata_drv_lock;

static void ata_io_wait(struct ata_device * dev) {
    inb(dev->io_base + ATA_REG_ALTSTATUS);
    inb(dev->io_base + ATA_REG_ALTSTATUS);
    inb(dev->io_base + ATA_REG_ALTSTATUS);
    inb(dev->io_base + ATA_REG_ALTSTATUS);
}


int ata_wait(struct ata_device * dev, int advanced) {
    uint8_t status = 0;

    ata_io_wait(dev);

    while ((status = inb(dev->io_base + ATA_REG_STATUS)) & ATA_SR_BSY) {
        //dbgpf("ATA: Status: %x\n", inb(dev->io_base + ATA_REG_STATUS));
        //yield();
    }

    if (advanced) {
        status = inb(dev->io_base + ATA_REG_STATUS);
        if (status   & ATA_SR_ERR) {
            dbgpf("ATA: ATA_SR_ERR\n");
            return 1;
        }
        if (status   & ATA_SR_DF)   {
            dbgpf("ATA: ATA_SR_DF\n");
            return 1;
        }
        if (!(status & ATA_SR_DRQ)) {
            dbgpf("ATA: ATA_SR_DRQ\n");
            return 1;
        }
    }

    return 0;
}

static void ata_soft_reset(struct ata_device * dev) {
    outb(dev->control, 0x04);
    outb(dev->control, 0x00);
}

static void ata_device_init(struct ata_device * dev) {

    dbgpf("ATA: Initializing IDE device on bus %d\n", dev->io_base);

    outb(dev->io_base + 1, 1);
    outb(dev->control, 0);

    outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
    ata_io_wait(dev);

    outb(dev->io_base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_wait(dev);

    int status = inb(dev->io_base + ATA_REG_COMMAND);
    dbgpf("ATA: Device status: %d\n", status);

    ata_wait(dev, 0);

    uint16_t * buf = (uint16_t *)&dev->identity;

    for (int i = 0; i < 256; ++i) {
        buf[i] = inw(dev->io_base);
    }

    uint8_t * ptr = (uint8_t *)&dev->identity.model;
    for (int i = 0; i < 39; i+=2) {
        uint8_t tmp = ptr[i+1];
        ptr[i+1] = ptr[i];
        ptr[i] = tmp;
    }

    dbgpf("ATA: Device Name:  %s\n", dev->identity.model);
    dbgpf("ATA: Sectors (48): %d\n", (uint32_t)dev->identity.sectors_48);
    dbgpf("ATA: Sectors (24): %d\n", dev->identity.sectors_28);

    outb(dev->io_base + ATA_REG_CONTROL, 0);
}

static int ata_device_detect(struct ata_device * dev) {
    ata_soft_reset(dev);
    outb(dev->io_base + ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
    ata_io_wait(dev);

    unsigned char cl = inb(dev->io_base + ATA_REG_LBA1); /* CYL_LO */
    unsigned char ch = inb(dev->io_base + ATA_REG_LBA2); /* CYL_HI */

    dbgpf("ATA: Device detected: 0x%2x 0x%2x\n", cl, ch);
    if (cl == 0xFF && ch == 0xFF) {
        /* Nothing here */
        return 0;
    }
    if (cl == 0x00 && ch == 0x00) {
        /* Parallel ATA device */
        ata_device_init(dev);
        char devicename[9]="ATA";
        // add_device(devicename, &ata_driver, (void*)dev);
        mbr_parse(devicename);
        return 1;
    }

    /* TODO: ATAPI, SATA, SATAPI */
    return 0;
}

/*
int32_t ata_device_read_sector(struct ata_device * dev, uint32_t lba, uint8_t * buf){
    AutoSpinLock l(&ata_lock);
    if(init_dma()){
        return dma_begin_read_sector(dev, lba, buf);
    }else{
        return -EFOPS;
    }
}

void ata_device_write_sector(struct ata_device * dev, uint32_t lba, uint8_t * buf) {
    spin_lock(&ata_lock);

    if(init_dma()){} // TODO: FIXME: Use DMA
    uint16_t bus = dev->io_base;
    uint8_t slave = dev->slave;

    outb(bus + ATA_REG_CONTROL, 0);

    ata_wait(dev, 0);
    outb(bus + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
    ata_wait(dev, 0);

    outb(bus + ATA_REG_SECCOUNT0, 0x01);
    outb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
    outb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
    outb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
    outb(bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    ata_wait(dev, 0);
    int size = ATA_SECTOR_SIZE / 2;
    outsm(bus,buf,size);
    outb(bus + 0x07, ATA_CMD_CACHE_FLUSH);
    ata_wait(dev, 0);
    spin_unlock(&ata_lock);
}

static int buffer_compare(uint32_t * ptr1, uint32_t * ptr2, size_t size) {
    if(size % 4) panic("ATA buffer compare size is not multiple of 4.");
    size_t i = 0;
    while (i < size) {
        if (*ptr1 != *ptr2) return 1;
        ptr1++;
        ptr2++;
        i += sizeof(uint32_t);
    }
    return 0;
}

void ata_device_write_sector_retry(struct ata_device * dev, uint32_t lba, uint8_t * buf) {
    uint8_t *read_buf = (uint8_t*) malloc(ATA_SECTOR_SIZE);
    disable_interrupts();
    do {
        ata_device_write_sector(dev, lba, buf);
        ata_device_read_sector(dev, lba, read_buf);
    } while (buffer_compare((uint32_t *)buf, (uint32_t *)read_buf, ATA_SECTOR_SIZE));
    enable_interrupts();
    free(read_buf);
}
*/

static struct ata_device ata_primary_master   = {.io_base = 0x1F0, .control = 0x3F6, .slave = 0};
static struct ata_device ata_primary_slave    = {.io_base = 0x1F0, .control = 0x3F6, .slave = 1};
static struct ata_device ata_secondary_master = {.io_base = 0x170, .control = 0x376, .slave = 0};
static struct ata_device ata_secondary_slave  = {.io_base = 0x170, .control = 0x376, .slave = 1};

template<ata_device* WhichDev>
class FOps : public IFOps
{
private:
    FOps() {}
public:
    ~FOps() {}

    virtual int32_t read(FsSpecificData *fdData, uint8_t *buf, int32_t bytes)
    {
        // Cannot read more than 1MB at one time, that's too much memory.
        if(bytes > 1024 * 1024) return -EFOPS;
        
        // TEST mode: the unit of fseek is LBA
        // TODO: FIXME: no seek. currently we always start from sector 0
        return dma_begin_read_sector(WhichDev, 0, buf, bytes);
        /*
        hold_lock hl(&ata_drv_lock);
        ata_instance *inst=(ata_instance*)instance;
        for(size_t i=0; i<bytes; i+=512){
            if(!cache_get((size_t)inst->dev, inst->pos/512, &buf[i])) {
                spin_unlock(&ata_drv_lock);
                ata_queued_read(inst->dev, inst->pos / 512, (uint8_t *) &buf[i]);
                spin_lock(&ata_drv_lock);
                cache_add((size_t)inst->dev, inst->pos/512, &buf[i]);
            }
            inst->pos+=512;
        }
        */
        return bytes;
    }

    virtual int32_t write(FsSpecificData *fdData, const uint8_t *buf, int32_t bytes)
    {
        // Currently read-only
        return -EFOPS;
        /*
        hold_lock hl(&ata_drv_lock);
        if(bytes % 512) return 0;
        ata_instance *inst=(ata_instance*)instance;
        for(size_t i=0; i<bytes; i+=512){
            cache_drop((size_t)inst->dev, inst->pos/512);
            spin_unlock(&ata_drv_lock);
            ata_queued_write(inst->dev, inst->pos/512, (uint8_t*)&buf[i]);
            spin_lock(&ata_drv_lock);
            inst->pos+=512;
        }
        return bytes;
        */
    }

    static Maybe<IFOps*> getNewInstance()
    {
        AutoSpinLock l(&ata_drv_lock);
        return reinterpret_cast<IFOps*>(new FOps);
    }
};

/*
size_t ata_seek(void *instance, size_t pos, uint32_t flags){
    ata_instance *inst=(ata_instance*)instance;
    if(pos % 512) return inst->pos;
    if(flags & FS_Relative) inst->pos+=pos;
    else if(flags & FS_Backwards){
        inst->pos = (inst->dev->identity.sectors_48 * 512) - pos;
    }else if(flags == (FS_Relative | FS_Backwards)) inst->pos-=pos;
    else inst->pos=pos;
    return inst->pos;
}
*/

}   // namespace ata

using namespace ata;
using namespace filesystem;

DEFINE_DRIVER_INIT(pata) {
    spin_lock_init(&ata_lock);
    spin_lock_init(&ata_drv_lock);
    // init_queue();

    /* Detect drives and mount them */
    ata_device_detect(&ata_primary_master);
    ata_device_detect(&ata_primary_slave);
    ata_device_detect(&ata_secondary_master);
    ata_device_detect(&ata_secondary_slave);

    preinit_dma();

    register_devfs("ata00", []() { return FOps<&ata_primary_master>::getNewInstance(); });
    register_devfs("ata01", []() { return FOps<&ata_primary_slave>::getNewInstance(); });
    register_devfs("ata10", []() { return FOps<&ata_secondary_master>::getNewInstance(); });
    register_devfs("ata11", []() { return FOps<&ata_secondary_slave>::getNewInstance(); });
}

DEFINE_DRIVER_REMOVE(pata) {
    ;
}

