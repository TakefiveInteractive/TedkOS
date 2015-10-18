#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"
#include "blink.h"

#define NULL 0
#define WAIT 200
uint8_t *vmem_base_addr;
uint8_t *mp1_set_video_mode (void);
void add_frames(uint8_t *, uint8_t *, int32_t);
void ece391_memset(void* memory, char c, int n);
int32_t ece391_memcpy(void* dest, const void* src, int32_t n);

uint8_t file0[] = "frame0.txt";
uint8_t file1[] = "frame1.txt";

/* Extern the externally-visible MP1 functions */
extern int mp1_ioctl(unsigned long arg, unsigned long cmd);
extern void mp1_rtc_tasklet(unsigned long trash);

static struct mp1_blink_struct blink_array[80*25];

int main(void)
{
    int rtc_fd, ret_val, i, garbage;
    struct mp1_blink_struct blink_struct;

    ece391_memset(blink_array, 0, sizeof(struct mp1_blink_struct)*80*25);

    if(mp1_set_video_mode() == NULL) {
        return -1;
    }

    rtc_fd = ece391_open((uint8_t*)"rtc");

    add_frames(file0, file1, rtc_fd);

    ret_val = 32;
    ret_val = ece391_write(rtc_fd, &ret_val, 4);

    for(i=0; i<WAIT; i++) {
        ece391_read(rtc_fd, &garbage, 4);
        mp1_rtc_tasklet(garbage);
    }

    blink_struct.on_char = 'I';
    blink_struct.off_char = 'M';
    blink_struct.on_length = 7;
    blink_struct.off_length = 6;
    blink_struct.location = 6*80+60;

    mp1_ioctl((unsigned long)&blink_struct, RTC_ADD);

    for(i=0; i<WAIT; i++) {
        ece391_read(rtc_fd, &garbage, 4);
        mp1_rtc_tasklet(garbage);
    }

    mp1_ioctl((40 << 16 | (6*80+60)), RTC_SYNC);

    for(i=0; i<WAIT; i++) {
        ece391_read(rtc_fd, &garbage, 4);
        mp1_rtc_tasklet(garbage);
    }

    blink_struct.location = 60;
    mp1_ioctl(i, RTC_REMOVE);

    for(i=0; i<80*25; i++) {
        ece391_read(rtc_fd, &garbage, 4);
        mp1_rtc_tasklet(garbage);
    }

    ece391_close(rtc_fd);

    return 0;
}

void
add_frames(uint8_t *f0, uint8_t *f1, int32_t rtc_fd)
{
    int32_t row, col, offset = 40, eof0 = 0, eof1 = 0, num_bytes;
    int32_t fd0, fd1;
    struct mp1_blink_struct blink_struct;
    uint8_t c0 = '0', c1 = '0';

    blink_struct.on_length = 15;
    blink_struct.off_length = 15;

    row = 0;

    if( (fd0 = ece391_open(f0)) < 0 ) {
        ece391_halt(-1);
    }
    if( (fd1 = ece391_open(f1)) < 0 ) {
        ece391_halt(-1);
    }

    while(eof0 == 0 || eof1 == 0) {
        col = 0;
        while(1) {

            if(c0 != '\n') {
                num_bytes = ece391_read(fd0, &c0, 1);
                if(num_bytes == 0) {
                    c0 = '\n';
                    eof0 = 1;
                }
            }

            if(c1 != '\n') {
                num_bytes = ece391_read(fd1, &c1, 1);
                if(num_bytes == 0) {
                    c1 = '\n';
                    eof1 = 1;
                }
            }

            if(c0 == '\n' && c1 == '\n') {
                break;

            } else {
                if((c0 != ' ' && c0 != '\n') || (c1 != ' ' && c1 != '\n')) {
                    blink_struct.on_char = ( (c0 == '\n') ? ' ' : c0);
                    blink_struct.off_char = ( (c1 == '\n') ? ' ' : c1);
                    blink_struct.location = row*80 + col + offset;
                    mp1_ioctl((unsigned long)&blink_struct, RTC_ADD);
                }
            }
            col++;
        }

        if(eof0) {
            c0 = '\n';
            ece391_close(fd0);
        } else {
            c0 = '0';
        }

        if(eof1) {
            c1 = '\n';
            ece391_close(fd1);
        } else {
            c1 = '0';
        }

        row++;
    }
}

uint8_t*
mp1_set_video_mode (void)
{
    if(ece391_vidmap(&vmem_base_addr) == -1) {
        return NULL;
    } else {
        return vmem_base_addr;
    }
}

void* mp1_malloc(int32_t size)
{
    int32_t i;
    for(i=0; i< 80*25; i++) {
        if(blink_array[i].location == 0) {
            return &blink_array[i];
        }
    }

    return NULL;
}

void mp1_free(void* memory)
{
    ece391_memset(memory, 0, sizeof(struct mp1_blink_struct));
}

void ece391_memset(void* memory, char c, int n)
{
    char* mem = (char*)memory;
    int i;
    for(i=0; i<n; i++) {
        mem[i] = c;
    }
}

int32_t ece391_memcpy(void* dest, const void* src, int32_t n)
{
    int32_t i;
    char* d = (char*)dest;
    char* s = (char*)src;
    for(i=0; i<n; i++) {
        d[i] = s[i];
    }

    return 0;
}
