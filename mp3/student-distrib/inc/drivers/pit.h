#ifndef _PIT_H_
#define _PIT_H_

#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/common.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/lib.h>
#include <inc/i8259.h>


const static int PIT_IRQ = 0;
const static int PIT_ID = 0;
const static int PIT_POLICY = 0;

const static int PIT_MAX_FREQ = 1193182;
const static int PIT_MIN_FREQ = 19;

const static int PIT_CMD_PORT = 0x43;

int pit_handler(int irq, unsigned int saved_reg);
int32_t pit_read (void* fd, uint8_t* buf, int32_t nbytes);
int pit_config(int channel, int mode, int freq);
int pit_init(int freq);

DEFINE_DRIVER_INIT(pit);
DEFINE_DRIVER_REMOVE(pit);

#endif
