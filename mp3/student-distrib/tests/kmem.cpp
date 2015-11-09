
#include <stdint.h>
#include <stddef.h>

#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

#define termputarr(FD, ARR) {fs_write((FD), (const void*)(ARR), sizeof(ARR)/sizeof(char));}

// Currently new "instance" only needs stdin and stdout
int kmem_main (int32_t term, int32_t kb)
{

    termputarr (term, "allocating and deallocating 4K char array for 128K times ...\n");
    for(int32_t i = 0; i < 128 * 1024; i++)
    {
        char** a = new char*[4*1024];
        for(int32_t j = 0; j < 4*1024; j++)
        {
            a[j] = new char;
        }
        for(int32_t j = 0; j < 4*1024; j++)
            delete a[j];
        delete[] a;
        printf("Done %d\n", i);
    }
    termputarr (term, "Success!\n");

    return 0;
}
