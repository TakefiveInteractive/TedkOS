#ifndef _PIT_H_
#define _PIT_H_

#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/common.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/lib.h>
#include <inc/i8259.h>

const static int32_t PIT_IRQ = 0;
const static int32_t PIT_ID = 0;
const static int32_t PIT_POLICY = 0;
const static uint32_t PIT_MAX_FREQ = 1193182;//hz
const static uint32_t PIT_MIN_FREQ = 19;//hz
const static int32_t PIT_CMD_PORT = 0x43;


int pit_handler(int irq, unsigned int saved_reg);
int pit_config(int channel, int mode, uint32_t freq);
int pit_init(uint32_t freq);
uint32_t pit_gettick();
uint32_t pit_tick2time(uint32_t tick);


DEFINE_DRIVER_INIT(pit);
DEFINE_DRIVER_REMOVE(pit);

#endif
