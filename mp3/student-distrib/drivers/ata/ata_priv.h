/**
 * Partially adapted from https://github.com/mallardtheduck/osdev/
 */

/*
 * This code is copied from https://github.com/mallardtheduck/osdev and slightly revised.
 */

#ifndef _INC_ATA_HPP_
#define _INC_ATA_HPP_

#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/function.h>

#define dbgpf printf
#define dbgout printf
#define osdev_outb(port, data) outb((data), (port))
#define osdev_outl(port, data) outl((data), (port))
#define osdev_outw(port, data) outw((data), (port))

inline void panic(const char* str) {
    printf(str);
    asm volatile("1: hlt; jmp 1b;");
}

namespace ata {

constexpr uint16_t ATA_SR_BSY      = 0x80;
constexpr uint16_t ATA_SR_DRDY     = 0x40;
constexpr uint16_t ATA_SR_DF       = 0x20;
constexpr uint16_t ATA_SR_DSC      = 0x10;
constexpr uint16_t ATA_SR_DRQ      = 0x08;
constexpr uint16_t ATA_SR_CORR     = 0x04;
constexpr uint16_t ATA_SR_IDX      = 0x02;
constexpr uint16_t ATA_SR_ERR      = 0x01;

constexpr uint16_t ATA_ER_BBK       = 0x80;
constexpr uint16_t ATA_ER_UNC       = 0x40;
constexpr uint16_t ATA_ER_MC        = 0x20;
constexpr uint16_t ATA_ER_IDNF      = 0x10;
constexpr uint16_t ATA_ER_MCR       = 0x08;
constexpr uint16_t ATA_ER_ABRT      = 0x04;
constexpr uint16_t ATA_ER_TK0NF     = 0x02;
constexpr uint16_t ATA_ER_AMNF      = 0x01;

constexpr uint16_t ATA_CMD_READ_PIO           = 0x20;
constexpr uint16_t ATA_CMD_READ_PIO_EXT       = 0x24;
constexpr uint16_t ATA_CMD_READ_DMA           = 0xC8;
constexpr uint16_t ATA_CMD_READ_DMA_EXT       = 0x25;
constexpr uint16_t ATA_CMD_WRITE_PIO          = 0x30;
constexpr uint16_t ATA_CMD_WRITE_PIO_EXT      = 0x34;
constexpr uint16_t ATA_CMD_WRITE_DMA          = 0xCA;
constexpr uint16_t ATA_CMD_WRITE_DMA_EXT      = 0x35;
constexpr uint16_t ATA_CMD_CACHE_FLUSH        = 0xE7;
constexpr uint16_t ATA_CMD_CACHE_FLUSH_EXT    = 0xEA;
constexpr uint16_t ATA_CMD_PACKET             = 0xA0;
constexpr uint16_t ATA_CMD_IDENTIFY_PACKET    = 0xA1;
constexpr uint16_t ATA_CMD_IDENTIFY           = 0xEC;

constexpr uint16_t ATAPI_CMD_READ        = 0xA8;
constexpr uint16_t ATAPI_CMD_EJECT       = 0x1B;

constexpr uint16_t ATA_IDENT_DEVICETYPE    = 0;
constexpr uint16_t ATA_IDENT_CYLINDERS     = 2;
constexpr uint16_t ATA_IDENT_HEADS         = 6;
constexpr uint16_t ATA_IDENT_SECTORS       = 12;
constexpr uint16_t ATA_IDENT_SERIAL        = 20;
constexpr uint16_t ATA_IDENT_MODEL         = 54;
constexpr uint16_t ATA_IDENT_CAPABILITIES  = 98;
constexpr uint16_t ATA_IDENT_FIELDVALID    = 106;
constexpr uint16_t ATA_IDENT_MAX_LBA       = 120;
constexpr uint16_t ATA_IDENT_COMMANDSETS   = 164;
constexpr uint16_t ATA_IDENT_MAX_LBA_EXT   = 200;

constexpr uint16_t IDE_ATA         = 0x00;
constexpr uint16_t IDE_ATAPI       = 0x01;

constexpr uint16_t ATA_MASTER      = 0x00;
constexpr uint16_t ATA_SLAVE       = 0x01;

constexpr uint16_t ATA_REG_DATA        = 0x00;
constexpr uint16_t ATA_REG_ERROR       = 0x01;
constexpr uint16_t ATA_REG_FEATURES    = 0x01;
constexpr uint16_t ATA_REG_SECCOUNT0   = 0x02;
constexpr uint16_t ATA_REG_LBA0        = 0x03;
constexpr uint16_t ATA_REG_LBA1        = 0x04;
constexpr uint16_t ATA_REG_LBA2        = 0x05;
constexpr uint16_t ATA_REG_HDDEVSEL    = 0x06;
constexpr uint16_t ATA_REG_COMMAND     = 0x07;
constexpr uint16_t ATA_REG_STATUS      = 0x07;
constexpr uint16_t ATA_REG_SECCOUNT1   = 0x08;
constexpr uint16_t ATA_REG_LBA3        = 0x09;
constexpr uint16_t ATA_REG_LBA4        = 0x0A;
constexpr uint16_t ATA_REG_LBA5        = 0x0B;
constexpr uint16_t ATA_REG_CONTROL     = 0x0C;
constexpr uint16_t ATA_REG_ALTSTATUS   = 0x0C;
constexpr uint16_t ATA_REG_DEVADDRESS  = 0x0D;

// Channels:
constexpr uint16_t ATA_PRIMARY       = 0x00;
constexpr uint16_t ATA_SECONDARY     = 0x01;

// Directions:
constexpr uint16_t ATA_READ       = 0x00;
constexpr uint16_t ATA_WRITE      = 0x01;

typedef struct {
    uint16_t base;
    uint16_t ctrl;
    uint16_t bmide;
    uint16_t nien;
} ide_channel_regs_t;

typedef struct {
    uint8_t  reserved;
    uint8_t  channel;
    uint8_t  drive;
    uint16_t type;
    uint16_t signature;
    uint16_t capabilities;
    uint32_t command_sets;
    uint32_t size;
    uint8_t  model[41];
} ide_device_t;

typedef struct {
    uint8_t  status;
    uint8_t  chs_first_sector[3];
    uint8_t  type;
    uint8_t  chs_last_sector[3];
    uint32_t lba_first_sector;
    uint32_t sector_count;
} partition_t;

typedef struct {
    uint16_t flags;
    uint16_t unused1[9];
    char     serial[20];
    uint16_t unused2[3];
    char     firmware[8];
    char     model[40];
    uint16_t sectors_per_int;
    uint16_t unused3;
    uint16_t capabilities[2];
    uint16_t unused4[2];
    uint16_t valid_ext_data;
    uint16_t unused5[5];
    uint16_t size_of_rw_mult;
    uint32_t sectors_28;
    uint16_t unused6[38];
    uint64_t sectors_48;
    uint16_t unused7[152];
} __attribute__((packed)) ata_identify_t;

typedef struct {
    uint8_t     boostrap[446];
    partition_t partitions[4];
    uint8_t     signature[2];
} __attribute__((packed)) mbr_t;

struct ata_device {
    int io_base;
    int control;
    int slave;
    ata_identify_t identity;
};

void ata_device_read_sector(struct ata_device * dev, uint32_t lba, uint8_t * buf);
void ata_device_read_sector_pio(struct ata_device * dev, uint32_t lba, uint8_t * buf);
void ata_device_write_sector_retry(struct ata_device * dev, uint32_t lba, uint8_t * buf);
void init_queue();
void ata_queued_read(ata_device *dev, uint32_t lba, uint8_t *buf);
void ata_queued_write(ata_device *dev, uint32_t lba, uint8_t *buf);

int ata_wait(struct ata_device * dev, int advanced);

void mbr_parse(char* device);

/* TODO support other sector sizes */
constexpr size_t ATA_SECTOR_SIZE = 512;

void cache_add(size_t deviceid, size_t sector, char *data);
bool cache_get(size_t deviceid, size_t sector, char *data);
void cache_drop(size_t deviceid, size_t sector);

void preinit_dma();
bool init_dma();
int32_t dma_begin_read_sector(ata_device *dev, uint32_t lba, uint8_t *buf, uint32_t nbytes, function<void ()> dadClassCallback);

}

#endif
