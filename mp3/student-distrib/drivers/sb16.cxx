#include <drivers/sb16.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/proc/tasks.h>
#include <inc/fs/fops.h>
#include <inc/fs/dev_wrapper.h>

using namespace filesystem;

/*
 * This driver is shamelessly adapted from
 * http://www.github.com/Mrjohns42/XiOS/
 *(mostly)
 * http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html#DSPPorts
 * (mostly)
 * and
 * http://www.inversereality.org/tutorials/sound%20programming/soundblaster16.html
 * (a little bit)
 */

#define SB_MODE_8BIT 0x10

#define READ_STATUS (1 << 7)
#define WRITE_STATUS (1 << 7)

#define LO_BYTE(data) ((data) & 0xFF)
#define HI_BYTE(data) (((data) >> 8) & 0xFF)

// DSP commands
enum {
    DSP_STOP_8BIT = 0xD0,
    DSP_STOP_16BIT = 0xD5,
    DSP_STOP_AFTER_8BIT = 0xDA,
    DSP_STOP_AFTER_16BIT = 0xD9,
    DSP_PLAY_8BIT = 0xC0,
    DSP_PLAY_16BIT = 0xB0,
    DSP_PAUSE_8BIT = 0xD0,
    DSP_RESUME_8BIT = 0xD4,
    DSP_PAUSE_16BIT = 0xD5,
    DSP_RESUME_16BIT = 0xD6,

    // DSP mode flags
    DSP_PLAY_AI = 0x06,
    DSP_PLAY_UNSIGNED = 0x00,
    DSP_PLAY_SIGNED = 0x10,
    DSP_PLAY_MONO = 0x00,
    DSP_PLAY_STEREO = 0x20,
};

enum {
    SB_SET_RATE = 0x41,
};

enum {
    // playback
    DMA_MODE_SC = 0x48,
    DMA_MODE_AI = 0x48 | 0x10,
    // recording
    DMA_MODE_RECORD_SC = 0x44,
    DMA_MODE_RECORD_AI = 0x44 | 0x10,
};

// general DSP ports
enum {
    DSPReset = 0x6,
    DSPRead = 0xA,
    DSPWrite = 0xC,
    DSPWriteStatus = 0xC,
    DSPReadStatus = 0xE,
    DSPIntAck = 0xF,
};

// DMA ports (1 is for 8-bit operations and 2 is for 16-bit)
enum {
    DMA1Mask = 0xA,
    DMA1Mode = 0xB,
    DMA1ClrBytePtr = 0xC,
    DMA2Mask = 0xD4,
    DMA2Mode = 0xD6,
    DMA2ClrBytePtr = 0xD8,
    DMA1Base = 0x00,
    DMA2Base = 0xC0,
};

// address ports (index with channel)
uint16_t addr_port[] = {0x0, 0x2, 0x4, 0x6,  // channel 0-3
    0xC0, 0xC4, 0xC8, 0xCC // chanel 4-7
};

// count ports (index with channel)
uint16_t count_port[] = {0x1, 0x3, 0x5, 0x7,  // channel 0-3
    0xC2, 0xC6, 0xCA, 0xCE // chanel 4-7
};

// page ports (index with channel)
static uint16_t page_port[] = {0x87, 0x83, 0x81, 0x82, // channel 0-3
   0x8F, // channel 4 is apparently unusable
   0x8B, 0x89, 0x8A // channel 5-7
};

uint8_t dma_inb(uint16_t port) {
    return inb(port);
}

#define CHUNK_SIZE (32*1024)
static int8_t dma_buffer[CHUNK_SIZE*2] __attribute__((aligned(64*1024)));
static int8_t *first_block = dma_buffer;
static int8_t *second_block = dma_buffer + CHUNK_SIZE;
static int8_t *current_block = dma_buffer;

#define STATUS_PLAYING (1 << 0)
#define STATUS_8BIT (1 << 1)
#define STATUS_SIGNED (1 << 2)
#define STATUS_MONO (1 << 3)

typedef struct playback_status {
    process_t *process;
    File fd;
    int8_t channel;
    int8_t mode;
    int8_t playing;
} playback_status_t;

static playback_status_t status;

playback_status_t *play_status = &status;

// strings as integers (for chunk ids)
// "RIFF"
#define RIFF 0x46464952
// "WAVE"
#define WAVE 0x45564157
// "fmt "
#define FMT 0x20746D66
// "data"
#define DATA 0x61746164
// "fact"
#define FACT 0x74636166

// forward declarations
typedef struct chunk_info {
    uint32_t chunk_id;
    uint32_t data_size;
} chunk_info_t;
void reset_playback();

chunk_info_t wav_read_chunk_header();

/*
 * output data to the SoundBlaster 16
 *
 * @param data byte to send to sb16
 * @param offset symbolic constant of offset from sb16 baseport
 */
void sb16_outb(uint8_t data, uint16_t offset) {
    static const int16_t baseport = 0x220;
    outb(data, baseport + offset);
}

/*
 * get a byte from the sb16
 *
 * @param offset symbolic constant of offset from sb16 baseport
 * @return byte from sb16
 */
uint8_t sb16_inb(uint16_t offset) {
    static const int16_t baseport = 0x220;
    return inb(baseport + offset);
}

void dma_outb(uint8_t data, uint16_t port) {
    outb(data, port);
}

void sb16_init() {
    sb16_reset();
    status.process = NULL;
    status.channel = 1;
    status.mode = 0;
    status.playing = 0;
}

unsigned int64_t inline getRDTSC() {
   __asm__ volatile {
      ; Flush the pipeline
      XOR eax, eax
      CPUID
      ; Get RDTSC counter in edx:eax
      RDTSC
   }
}

void sb16_reset() {
    sb16_outb(1, DSPReset);
    // need to wait 3 microseconds
    // assuming clock speed is at most 6Ghz
    int64_t start = getRDTSC();
    while (getRDTSC() - start < 0x480000000LL);

    sb16_outb(0, DSPReset);

    while ( !(sb16_inb(DSPReadStatus) & READ_STATUS) );
    while ( !(sb16_inb(DSPRead) == 0xAA) );
}

void enable_dma(uint32_t dmanr) {
    if (dmanr <= 3)
        dma_outb(dmanr, DMA1Mask);
    else
        dma_outb(dmanr & 3, DMA2Mask);
}

void disable_dma(uint32_t dmanr) {
    if (dmanr <= 3)
        dma_outb(dmanr | 4, DMA1Mask);
    else
        dma_outb((dmanr & 3) | 4, DMA2Mask);
}

void clear_dma_ff(uint32_t dmanr) {
    if (dmanr <= 3)
        dma_outb(0, DMA1ClrBytePtr);
    else
        dma_outb(0, DMA2ClrBytePtr);
}

void set_dma_mode(uint32_t dmanr, char mode) {
    if (dmanr <= 3)
        dma_outb(mode | dmanr, DMA1Mode);
    else
        dma_outb(mode | (dmanr & 3), DMA2Mode);
}

// don't know why this is here, it's from linux-2.6/arch/x86/include/asm/dma.h
void set_dma_page(uint32_t dmanr, char pagenr) {
    if (dmanr <= 3)
        dma_outb(pagenr, page_port[dmanr]);
    else
        dma_outb(pagenr & 0xFE, page_port[dmanr]);
}

void set_dma_addr(uint32_t dmanr, uint32_t addr) {
    set_dma_page(dmanr, addr >> 16);
    if (dmanr <= 3)  {
        dma_outb( LO_BYTE(addr), ((dmanr & 3) << 1) + DMA1Base);
        dma_outb( HI_BYTE(addr), ((dmanr & 3) << 1) + DMA1Base);
    } else  {
        dma_outb(LO_BYTE(addr/2), ((dmanr & 3) << 2) + DMA2Base);
        dma_outb(HI_BYTE(addr/2), ((dmanr & 3) << 2) + DMA2Base);
    }
}

void set_dma_count(uint32_t dmanr, uint32_t count) {
    count--;
    if (dmanr <= 3)  {
        dma_outb( LO_BYTE(count), ((dmanr & 3) << 1) + 1 + DMA1Base);
        dma_outb( HI_BYTE(count),
                ((dmanr & 3) << 1) + 1 + DMA1Base);
    } else {
        dma_outb( LO_BYTE(count/2),
                ((dmanr & 3) << 2) + 2 + DMA2Base);
        dma_outb( HI_BYTE(count/2),
                ((dmanr & 3) << 2) + 2 + DMA2Base);
    }
}

void dma_start(uint8_t channel, uint32_t addr, uint32_t size, int8_t mode) {
    if (channel >= 8) {
        return;
    }
    // Disable the sound card DMA channel by setting the appropriate mask bit
    disable_dma(channel);
    // Clear the byte pointer flip-flop
    clear_dma_ff(channel);
    // Write the DMA mode for the transfer (48h is single-cycle playback)
    set_dma_mode(channel, mode);
    // Write the offset of the buffer, low byte followed by high byte.
    set_dma_addr(channel, addr);
    // Write the transfer length, low byte followed by high byte.
    set_dma_count(channel, size);
    // Enable the sound card DMA channel by clearing the appropriate mask bit
    enable_dma(channel);
}

void sb16_start_playback(uint32_t size) {
    uint16_t block_size = (uint16_t) size;
    // write the I/O command, transfer mode and block size to DSP
    uint8_t command = 0;
    if (status.mode & STATUS_8BIT) {
        command |= DSP_PLAY_8BIT;
    } else {
        command |= DSP_PLAY_16BIT;
    }
    command |= DSP_PLAY_AI;
    sb16_dsp_write(command);
    uint8_t mode = 0;
    if (status.mode & STATUS_MONO) {
        mode |= DSP_PLAY_MONO;
    }
    if (status.mode & STATUS_SIGNED) {
        mode |= DSP_PLAY_SIGNED;
    }
    sb16_dsp_write(mode);
    block_size--;
    sb16_dsp_write(LO_BYTE(block_size));
    sb16_dsp_write(HI_BYTE(block_size));
}

int32_t sb16_pause_playback() {
    if (!status.playing) {
        return -1;
    }
    if (status.mode & STATUS_8BIT) {
        sb16_dsp_write(DSP_PAUSE_8BIT);
    } else {
        sb16_dsp_write(DSP_PAUSE_16BIT);
    }
    return 0;
}

int32_t sb16_resume_playback() {
    if (!status.playing) {
        return -1;
    }
    if (status.mode & STATUS_8BIT) {
        sb16_dsp_write(DSP_RESUME_8BIT);
    } else {
        sb16_dsp_write(DSP_RESUME_16BIT);
    }
    return 0;
}

void sb16_stop_playback() {
    if (status.mode & STATUS_8BIT) {
        sb16_dsp_write(DSP_STOP_8BIT);
    } else {
        sb16_dsp_write(DSP_STOP_16BIT);
    }
    reset_playback();
}

void sb16_stop_playback_after() {
    if (status.mode & STATUS_8BIT) {
        sb16_dsp_write(DSP_STOP_AFTER_8BIT);
    } else {
        sb16_dsp_write(DSP_STOP_AFTER_16BIT);
    }
}

void sb16_set_sample_rate(uint16_t hz) {
    sb16_dsp_write(SB_SET_RATE);
    sb16_dsp_write( HI_BYTE(hz) );
    sb16_dsp_write( LO_BYTE(hz) );
}

chunk_info_t wav_read_chunk_header() {
    chunk_info_t info;
    info.chunk_id = 0;
    info.data_size = 0;
    uint8_t buf[13] = {0};
    // read in the chunk header
    if (theDispatcher->read(status.fd, buf, 8) < 8) {
        return info;
    }
    info.chunk_id = *(uint32_t *) (buf + 0);
    info.data_size = *(uint32_t *) (buf + 4);
    if (info.chunk_id == RIFF) {
        theDispatcher->read(status.fd, buf + 8, 4);
        if (*(uint32_t *) (buf + 8) != WAVE) {
            // RIFF chunk is invalid
            info.chunk_id = 0;
            info.data_size = 0;
            return info;
        }
    }
    return info;
}

uint16_t get_int16(void *buf, int offset) {
    return *(uint16_t*) (buf + offset);
}

uint32_t get_int32(void *buf, int offset) {
    return *(uint32_t*) (buf + offset);
}

// Parsing wave file information from
// http://www.sonicspot.com/guide/wavefiles.html

int32_t play_wav(int8_t *filename) {
    File& wavFile = status.fd;
    if (status.playing || filename == NULL) {
        return -1;
    }
    status.process = kernel_proc;
    theDispatcher->open(wavFile, filename);
    int32_t bytes_read;
    chunk_info_t info;
    info = wav_read_chunk_header();
    if (info.chunk_id != RIFF) {
        theDispatcher->close(wavFile);
        return -1;
    }
    info = wav_read_chunk_header();
    int8_t bits;
    int8_t is_signed;
    uint16_t sample_rate;
    if (info.chunk_id != FMT) {
        theDispatcher->close(wavFile);
        return -1;
    }
    uint8_t *format = new uint8_t[info.data_size];
    bytes_read = theDispatcher->read(status.fd, format, info.data_size);
    if (bytes_read < info.data_size) {
        delete [] format;
        theDispatcher->close(wavFile);
        return -1;
    }
    if (get_int16(format, 0) != 1) {
        // not uncompressed PCM format
        delete [] format;
        theDispatcher->close(wavFile);
        return -1;
    }
    if (get_int16(format, 0x2) != 1) {
        // not mono sound
        delete [] format;
        theDispatcher->close(wavFile);
        return -1;
    }
    bits = get_int16(format, 0xE);
    if (bits == 16) {
        is_signed = 1;
    } else {
        is_signed = 0;
    }
    sample_rate = get_int16(format, 0x4);
    delete [] format;

    info = wav_read_chunk_header();
    if (info.chunk_id == FACT) {
        uint8_t *buf = new uint8_t[info.data_size];
        bytes_read = theDispatcher->read(status.fd, buf, info.data_size);
        delete [] buf;
        info = wav_read_chunk_header();
    }
    if (info.chunk_id != DATA) {
        // no data? oh no
        theDispatcher->close(wavFile);
        current_process = old_process;
        return -1;
    }
    current_process = old_process;
    return play_file(wavFile, bits, is_signed, sample_rate);
}

int32_t play_file(File& fd, int8_t bits, int8_t is_signed, uint16_t sample_rate) {
    if (status.playing) {
        return -1;
    }
    if (bits == 8) {
        status.mode |= STATUS_8BIT;
        status.channel = 1;
    } else if (bits != 16) {
        // can only play 8 or 16bit audio
        return -1;
    } else {
        status.channel = 5;
    }

    status.process = kernel_proc;
    status.playing = 1;
    if (is_signed) {
        status.mode |= STATUS_SIGNED;
    }

    // set sampling rate
    sb16_set_sample_rate(sample_rate);

    // process_t *old_process = current_process;
    // current_process = status.process;
    uint32_t bytes_read = theDispatcher->read(fd, (uint8_t*)dma_buffer, CHUNK_SIZE*2);
    uint32_t bytes_read = syscall_read(fd, (uint8_t*)dma_buffer, CHUNK_SIZE*2);
    // current_process = old_process;
    dma_start(status.channel, (uint32_t) dma_buffer, sizeof(dma_buffer), DMA_MODE_AI);
    sb16_start_playback((uint16_t) CHUNK_SIZE);
    if (bytes_read < CHUNK_SIZE) {
        sb16_stop_playback_after();
    }
    return 0;
}

void sb16_dsp_write(uint8_t byte) {
    // Read the write-buffer status port (2xC) until bit 7 is cleared
    while (sb16_inb(DSPWriteStatus) & WRITE_STATUS);
    // Write the value to the write port (2xC)
    sb16_outb(byte, DSPWrite);
}

uint8_t sb16_dsp_read() {
    while ((sb16_inb(DSPReadStatus) & READ_STATUS) == 0);
    // Read the value
    return sb16_inb(DSPRead);
}

void reset_playback() {
    status.playing = 0;
    current_block = first_block;
    int i;
    for (i = 0; i < sizeof(dma_buffer); i++) {
        dma_buffer[i] = 0;
    }
    theDispatcher->close(status.fd);
}

void sb16_handler() {
    //save all
    registers_t regs;
    save_regs(regs);

    if (status.playing) {
        process_t *old_process = current_process;
        current_process = status.process;
        uint32_t flags;
        block_interrupts(&flags);
        uint32_t bytes_read = syscall_read(status.fd, (uint8_t*)current_block, CHUNK_SIZE);
        restore_interrupts(flags);
        current_process = old_process;
        if (bytes_read > 0) {
            uint16_t block_size = (uint16_t) bytes_read;
            // there are no more chunks after this one; stop playback after the
            // current block
            if (bytes_read < CHUNK_SIZE) {
                sb16_stop_playback_after();
                reset_playback();
            }
            if (current_block == first_block) {
                current_block = second_block;
            } else {
                current_block = first_block;
            }

            sb16_start_playback(block_size);
        } else {
            // file has no more bytes left; we're done playing
            reset_playback();
            sb16_stop_playback();
        }
    }

    sb16_inb(DSPReadStatus);
    sb16_inb(DSPIntAck);
}

DEFINE_DRIVER_INIT(sb16)
{
    bind_irq(SB16_IRQ, SB16_ID, sb16_handler, SB16_POLICY);
    sb16_init();
    filesystem::register_devfs("sb16", []() { return FOpsRTC::getNewInstance(); });
    return;
}

DEFINE_DRIVER_REMOVE(sb16)
{
    unbind_irq(SB16_IRQ, SB16_ID);
    return;
}
