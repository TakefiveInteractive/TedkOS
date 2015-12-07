#ifndef _SB16_H_
#define _SB16_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/drivers/common.h>
#include <inc/klibs/lib.h>
#include <inc/i8259.h>
#include <inc/fs/filesystem.h>
#include <inc/syscalls/filesystem_wrapper.h>

using namespace filesystem;

#define SB16_IRQ 5
#define SB16_ID 123 
#define SB16_POLICY 0

void sb16_outb(uint8_t data, uint16_t offset);
uint8_t sb16_inb(uint16_t offset);
void dma_outb(uint8_t data, uint16_t port);
uint8_t dma_inb(uint16_t port);
void sb16_init();
void sb16_reset();
void dma_start(uint8_t channel, uint32_t addr, uint32_t size, int8_t mode);
void sb16_dsp_write(uint8_t byte);
uint8_t sb16_dsp_read();
int sb16_handler(int irq, unsigned int saved_reg);
// void sb16_handler();
int32_t play_wav(char *filename);
int32_t play_file(File& fd, int8_t bits, int8_t is_signed, uint16_t sample_rate);
int32_t sb16_pause_playback();
int32_t sb16_resume_playback();
void sb16_stop_playback();

#ifdef __cplusplus
extern "C"{
#endif
extern DEFINE_DRIVER_INIT(sb16);
extern DEFINE_DRIVER_REMOVE(sb16);
#ifdef __cplusplus
}
#endif

//extern int8_t dma_buffer[128*1024];
//extern int32_t soundfd;
#endif
