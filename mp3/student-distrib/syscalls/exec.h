#ifndef _SYSCALL_EXEC
#define _SYSCALL_EXEC

#include <stdint.h>
#include <stddef.h>

namespace syscall_exec {

const uint32_t code_page_vaddr_base = 0x08000000;
const uint32_t program_image_offset = 0x00048000;
const uint32_t program_image_addr   = code_page_vaddr_base + program_image_offset;

// Check whether exists, and whether executable.
// Return 0 if not exist or not an executable.
int8_t is_kiss_executable(const uint8_t* file);

//   Load "file", an executable, to "program_image_addr"
//   You can assume that CR3 already points the page table of this process.
// Return 0 if success. Return -ENOMEM if file is too large (not possible, though)
void* kiss_loader(const uint8_t* file);

// Main entry to implementation of exec syscall
int32_t sysexec(const uint8_t* file);

int32_t do_exec(const uint8_t* file);

}

#endif /* _SYSCALL_EXEC */
