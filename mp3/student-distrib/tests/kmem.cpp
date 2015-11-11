
#include <stdint.h>
#include <stddef.h>

#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

#define termputarr(FD, ARR) {fs_write((FD), (const void*)(ARR), sizeof(ARR)/sizeof(char));}

// Currently new "instance" only needs stdin and stdout
int kmem_main (int32_t term, int32_t kb)
{

    termputarr (term, "allocating and deallocating 64K char array for 8K times ...\n");
    for(int32_t i = 0; i < 8 * 1024; i++)
    {
        char** a = new char*[64*1024];
        char *b = new char[128*1024];
        char**c = new char*[64*100];
        for (size_t x = 0; x < 128*1024; x++)
        {
            b[x] = x % 100;
        }
        for(int32_t j = 0; j < 64*1024; j++)
        {
            a[j] = new char;
        }
        for(int32_t j = 0; j < 64*1024; j++)
            delete a[j];
        delete[] a;
        for (size_t x = 0; x < 64*100; x++)
        {
            int max = x % 20000 + 100;
            c[x] = new char[max];
            for (int y = 0; y < max; y++)
            {
                c[x][y] = y % 100;
            }
        }
        for (int x = 0; x < 128*1024; x++)
        {
            if (b[x] != x % 100) { x = 0; x /= x; }
        }
        for (int x = 0; x < 64*100; x++)
        {
            int max = x % 20000 + 100;
            for (int y = 0; y < max; y++)
            {
                if (c[x][y] != y % 100) { x = 0; x /= x; }
            }
            delete[] c[x];
        }
        delete[] b;
        delete[] c;
        if(i % 1024 == 0 && i != 0)
            printf("Done %dK times\n", i/1024);
    }
    termputarr (term, "Success!\n");

    return 0;
}
