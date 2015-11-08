
#include <stdint.h>
#include <stddef.h>

#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

#define termputarr(FD, ARR) {fs_write((FD), (const void*)(ARR), sizeof(ARR)/sizeof(char));}

// Currently new "instance" only needs stdin and stdout
int kmem_main (int32_t term, int32_t kb)
{

    termputarr (term, "allocating and deallocating one char for 512M times ...\n");
    for(int32_t i = 0; i < 512 * 1024 * 1024; i++)
    {
        /*
        char** a = new char*[128*1024];
        for(int32_t j = 0; j < 128*1024; j++)
            a[j] = new char;
        for(int32_t j = 0; j < 128*1024; j++)
            delete a[j];
        delete[] a;
        */
        char* b =new char;
        delete b;
    }
    termputarr (term, "Success!\n");

    return 0;
}
