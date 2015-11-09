#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

//-------------------------------------------------------------------------
// Data declarations

typedef struct pheader_list_t {
    struct pheader_list_t *prev;
    struct pheader_list_t *next;
    uint32_t data;
} pheader_list_t;

pheader_list_t pheader_list = { .prev = &pheader_list, .next = &pheader_list, .data = 0 };
char magic_2882[5] = { 0x7f, 0x45, 0x4c, 0x46 };

//-------------------------------------------------------------------------
// Function declarations

int  verify_magic(const char *s2);
int  print_usage();
signed int  verify_ident(int a1);
void * read_program_header(int fd, int a2, int a3);
int  parse_program_header(void *a1);
void * list_insert_after(int a1, int a2, int a3);
void * list_insert_before(int a1, int a2, int a3);
int  list_remove(int, void *ptr); // idb
int  list_remove_tail(int a1);
int  list_remove_head(int a1);
void * list_insert_at_head(int a1, int a2);
void * list_insert_at_tail(int a1, int a2);

typedef uint32_t _DWORD;
typedef uint16_t _WORD;
typedef uint8_t _BYTE;
typedef int16_t __int16;

#define DWORD uint32_t
#define WORD uint16_t

#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w) ((BYTE)(w))
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))


int32_t inputbuf[128 * 1024 * 1024] = { };

//----- (08048674) --------------------------------------------------------
int  main(int argc, char *argv[])
{
    ssize_t v4; // eax@37
    const char *v7; // [sp+4h] [bp-74h]@24
    signed int v8; // [sp+8h] [bp-70h]@13
    unsigned int v9; // [sp+Ch] [bp-6Ch]@24
    char v10; // [sp+1Bh] [bp-5Dh]@24
    int v12; // [sp+34h] [bp-44h]@13
    int v13; // [sp+38h] [bp-40h]@13
    int v14; // [sp+3Ch] [bp-3Ch]@13
    int v15; // [sp+40h] [bp-38h]@13
    int v16; // [sp+44h] [bp-34h]@13
    int v17; // [sp+48h] [bp-30h]@13
    int v18; // [sp+4Ch] [bp-2Ch]@13
    int fd; // [sp+54h] [bp-24h]@4
    int v21; // [sp+58h] [bp-20h]@24
    int i; // [sp+5Ch] [bp-1Ch]@13
    int v23; // [sp+60h] [bp-18h]@14
    int j; // [sp+64h] [bp-14h]@24
    void *ptr; // [sp+68h] [bp-10h]@25
    size_t n; // [sp+6Ch] [bp-Ch]@36
    size_t v27; // [sp+70h] [bp-8h]@25
    size_t v28; // [sp+78h] [bp+0h]@29

    if ( argc != 2 )
    {
        print_usage();
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if ( read(fd, inputbuf, 0x34u) != 52 )
    {
        perror("Reading elf header");
        exit(1);
    }
    if ( verify_magic(inputbuf) )
    {
        fprintf(stderr, "%s is not an ELF file.\n", argv[1]);
        exit(1);
    }
    if ( verify_ident((int)&inputbuf) )
    {
        fprintf(stderr, "%s is not a compatible ELF file.\n", argv[1]);
        exit(1);
    }

    v12 = inputbuf[0x18 / 4];
    v13 = inputbuf[0x1C / 4];
    v14 = inputbuf[0x20 / 4];
    v15 = inputbuf[0x24 / 4];
    v16 = inputbuf[0x28 / 4];
    v17 = inputbuf[0x2A / 4];
    v18 = inputbuf[0x2C / 4];

    fprintf(stderr, "e_entry    : 0x%08x\n", v12);
    fprintf(stderr, "e_phoff    : 0x%08x\n", v13);
    fprintf(stderr, "e_shoff    : 0x%08x\n", v14);
    fprintf(stderr, "e_flags    : 0x%08x\n", v15);
    fprintf(stderr, "e_ehsize   : 0x%08x\n", (uint16_t)v16);
    fprintf(stderr, "e_phentsize: 0x%08x\n", HIWORD(v16));
    fprintf(stderr, "e_phnum    : 0x%08x\n", (uint16_t)v17);
    fprintf(stderr, "e_shentsize: 0x%08x\n", HIWORD(v17));
    v8 = (uint16_t)v18;
    fprintf(stderr, "e_shnum    : 0x%08x\n", (uint16_t)v18);
    fprintf(stderr, "\n", v8);
    for ( i = 0; (uint16_t)v17 > i; ++i )
    {
        void *header = read_program_header(fd, (int)&inputbuf, i);
        if ( !header )
        {
            fprintf(stderr, "Error reading header %d, skipping...\n", i);
            exit(1);
        }
        if ( parse_program_header(header) )
            list_insert_at_tail((int)&pheader_list, header);
    }
    if ( !i )
    {
        fprintf(stderr, "No loadable segments found...\n");
        exit(1);
    }
    char file[80] = { };
    strcpy(file, argv[1]);
    strcat(file, ".converted");
    v8 = 33188;
    v21 = open(file, 0x42, 0x81a4);
    for ( j = *(_DWORD *)&pheader_list; (char *)j != &pheader_list; j = *(_DWORD *)j )
    {
        v23 = *(_DWORD *)(j + 8);
        v27 = *(_DWORD *)(v23 + 20);
        ptr = malloc(v27);
        if ( lseek(fd, *(_DWORD *)(v23 + 4), 0) == -1 )
        {
            perror("lseek for pheader read");
            exit(1);
        }
        if ( *(_DWORD *)(v23 + 16) )
        {
            v28 = *(_DWORD *)(v23 + 16);
            if ( v28 != read(fd, ptr, v28) )
            {
                perror("pheader read");
                exit(1);
            }
        }
        if ( lseek(v21, *(_DWORD *)(v23 + 8) - 0x8048000, 0) == -1 )
        {
            perror("lseek for pheader write");
            exit(1);
        }
        if ( *(_DWORD *)(v23 + 20) > *(_DWORD *)(v23 + 16) )
        {
            n = *(_DWORD *)(v23 + 20) - *(_DWORD *)(v23 + 16);
            memset((char *)ptr + *(_DWORD *)(v23 + 16), 0, n);
        }
        v28 = *(_DWORD *)(v23 + 20);
        v4 = write(v21, ptr, v28);
        if ( v4 != *(_DWORD *)(v23 + 20) )
        {
            perror("pheader write");
            exit(1);
        }
        free(ptr);
    }
    return 0;
}

//----- (08048B7E) --------------------------------------------------------
int  verify_magic(const char *s2)
{
    return strncmp(magic_2882, s2, 4u);
}

//----- (08048BA2) --------------------------------------------------------
int  print_usage()
{
    return fprintf(stderr, "Usage: elfconvert <exename>\n");
}

//----- (08048BBF) --------------------------------------------------------
signed int  verify_ident(int a1)
{
    signed int v2; // [sp+0h] [bp-4h]@7

    if ( *(_BYTE *)(a1 + 4) != 1
            || *(_BYTE *)(a1 + 5) != 1
            || *(_BYTE *)(a1 + 6) != 1
            || *(_WORD *)(a1 + 16) != 2
            || *(_WORD *)(a1 + 18) != 3
            || *(_DWORD *)(a1 + 20) != 1 )
        v2 = -1;
    else
        v2 = 0;
    return v2;
}

//----- (08048C20) --------------------------------------------------------
void * read_program_header(int fd, int input, int idx)
{
    void *v4; // [sp+14h] [bp-14h]@2
    void *buf; // [sp+24h] [bp-4h]@3

    if ( lseek(fd, *(_DWORD *)(input + 28) + idx * *(_WORD *)(input + 42), 0) == -1 )
    {
        perror("lseek to program header");
        v4 = 0;
    }
    else
    {
        buf = malloc(0x20u);
        if ( read(fd, buf, *(_WORD *)(input + 42)) == *(_WORD *)(input + 42) )
        {
            v4 = buf;
        }
        else
        {
            perror("read program header");
            v4 = 0;
        }
    }
    return v4;
}

//----- (08048CCD) --------------------------------------------------------
int  parse_program_header(void *a1)
{
    int v2; // [sp+14h] [bp-4h]@2

    if ( *(_DWORD *)a1 == 1 )
    {
        fprintf(stderr, "p_type  : 0x%08x\n", *(_DWORD *)a1);
        fprintf(stderr, "p_offset: 0x%08x\n", *(_DWORD *)(a1 + 4));
        fprintf(stderr, "p_vaddr : 0x%08x\n", *(_DWORD *)(a1 + 8));
        fprintf(stderr, "p_paddr : 0x%08x\n", *(_DWORD *)(a1 + 12));
        fprintf(stderr, "p_filesz: 0x%08x\n", *(_DWORD *)(a1 + 16));
        fprintf(stderr, "p_memsz : 0x%08x\n", *(_DWORD *)(a1 + 20));
        fprintf(stderr, "p_flags : 0x%08x\n", *(_DWORD *)(a1 + 24));
        fprintf(stderr, "p_align : 0x%08x\n", *(_DWORD *)(a1 + 28));
        fprintf(stderr, "\n");
        v2 = *(_DWORD *)(a1 + 8);
    }
    else
    {
        v2 = 0;
    }
    return v2;
}

//----- (08048E0C) --------------------------------------------------------
void * list_insert_after(int a1, int a2, int a3)
{
    void *result; // eax@1

    ++*(_DWORD *)(a1 + 12);
    result = malloc(0xCu);
    *((_DWORD *)result + 2) = a3;
    *(_DWORD *)result = *(_DWORD *)a2;
    *((_DWORD *)result + 1) = a2;
    *(_DWORD *)(*(_DWORD *)a2 + 4) = result;
    *(_DWORD *)a2 = result;
    return result;
}

//----- (08048E64) --------------------------------------------------------
void * list_insert_before(int a1, int a2, int a3)
{
    void *result; // eax@1

    ++*(_DWORD *)(a1 + 12);
    result = malloc(0xCu);
    *((_DWORD *)result + 2) = a3;
    *(_DWORD *)result = a2;
    *((_DWORD *)result + 1) = *(_DWORD *)(a2 + 4);
    **(_DWORD **)(a2 + 4) = result;
    *(_DWORD *)(a2 + 4) = result;
    return result;
}

//----- (08048EBE) --------------------------------------------------------
int  list_remove(int a1, void *ptr)
{
    int v2; // ST14_4@1

    v2 = *((_DWORD *)ptr + 2);
    *(_DWORD *)(*(_DWORD *)ptr + 4) = *((_DWORD *)ptr + 1);
    **((_DWORD **)ptr + 1) = *(_DWORD *)ptr;
    free(ptr);
    return v2;
}

//----- (08048EF8) --------------------------------------------------------
int  list_remove_tail(int a1)
{
    return list_remove(a1, *(void **)(a1 + 4));
}

//----- (08048F15) --------------------------------------------------------
int  list_remove_head(int a1)
{
    return list_remove(a1, *(void **)a1);
}

//----- (08048F31) --------------------------------------------------------
void * list_insert_at_head(int a1, int a2)
{
    return list_insert_after(a1, a1, a2);
}

//----- (08048F52) --------------------------------------------------------
void * list_insert_at_tail(int a1, int a2)
{
    return list_insert_before(a1, a1, a2);
}

