#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

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

//-------------------------------------------------------------------------
// Data declarations

char bblock[4096]; // weak

//-------------------------------------------------------------------------
// Function declarations

typedef struct inode_list_t {
    struct inode_list_t *prev;
    struct inode_list_t *next;
    void *data;
} inode_list_t;

typedef struct inode_root_t {
    struct inode_list_t *head;
    struct inode_list_t *tail;
    uint32_t data;
    uint32_t count;
} inode_root_t;

inode_root_t inode_list = { .head = (inode_list_t *) &inode_list, .tail = (inode_list_t *) &inode_list, .data = 0, .count = 0 };

int32_t main(int32_t a1, char * a2[]);
int32_t create_dentry(const char *src, struct stat* a2, void *dest);
void *create_file(const char *file, struct stat* a2);
void *create_device();
void* list_insert_after(inode_root_t *a1, inode_list_t *element, void *data);
void* list_insert_before(inode_root_t *a1, inode_list_t *element, void *data);
void* list_insert_at_head(inode_root_t *root, void *data);
void* list_insert_at_tail(inode_root_t *root, void *data);
int32_t init_freemap(int32_t a1, int32_t a2);
int32_t get_inode_block(int32_t a1);
int32_t get_data_block(int32_t a1);
int32_t get_size(int32_t a1);
int32_t get_num_datablocks(int32_t a1);
int32_t get_num_inodes(int32_t a1);

//----- (080488B4) --------------------------------------------------------
int32_t main(int32_t a1, char *argv[])
{
  size_t v3; // eax@5
  size_t v4; // ebx@18
  size_t v5; // ebx@18
  size_t v6; // eax@18
  struct stat v8; // [sp+10h] [bp-98h]@18
  char *ptr; // [sp+74h] [bp-34h]@18
  void *data; // [sp+78h] [bp-30h]@24
  int32_t v11; // [sp+7Ch] [bp-2Ch]@21
  int32_t v12; // [sp+80h] [bp-28h]@33
  inode_list_t *i; // [sp+84h] [bp-24h]@33
  void *buf; // [sp+88h] [bp-20h]@35
  struct dirent *v15; // [sp+8Ch] [bp-1Ch]@17
  DIR *dirp; // [sp+90h] [bp-18h]@12
  int32_t fd; // [sp+94h] [bp-14h]@8
  char *src; // [sp+98h] [bp-10h]@12
  char *file; // [sp+9Ch] [bp-Ch]@2

  if ( a1 == 2 )
  {
    file = strdup("fs.out");
  }
  else
  {
    if ( a1 != 4 || strcmp(argv[2], "-o") )
    {
      fprintf(stderr, "Usage: fscreate <directory> [-o <output file>]\n");
      exit(1);
    }
    v3 = strlen(argv[3]);
    file = (char *)malloc(v3 + 4);
    strcpy(file, argv[3]);
  }
  fd = open(file, O_EXCL | O_CREAT | O_SYNC | O_WRONLY,
        S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  if ( fd == -1 )
  {
    fprintf(stderr, "open: Output file %s exists, do you want to overwrite?\n(y/n): ", file);
    if ( getchar() == 110 )
      exit(1);
    fd = open(file, O_CREAT | O_TRUNC | O_SYNC | O_WRONLY,
        S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  }
  src = argv[1];
  dirp = opendir(src);
  if ( !dirp )
    fprintf(stderr, "opendir: Directory %s does not exist\n", src);
  memset(bblock, 0, 0x1000u);
  strcpy(&bblock[64], ".");
  *(_DWORD *)&bblock[96] = 1;
  *(_DWORD *)&bblock[100] = 0;
  *(_DWORD *)bblock = 1;
  while ( dirp && *(_DWORD *)bblock <= 0x3Eu )
  {
    errno = 0;
    v15 = readdir(dirp);
    if ( !v15 )
    {
      if ( errno )
      {
        perror("readdir");
        exit(1);
      }
      closedir(dirp);
      break;
    }
    v4 = strlen(src);
    v5 = strlen("/") + v4;
    v6 = strlen(v15->d_name);
    ptr = (char *)malloc(v6 + v5 + 1);
    strcpy(ptr, src);
    strcat(ptr, "/");
    strcat(ptr, v15->d_name);
    if ( stat(ptr, &v8) )
    {
      perror("stat");
      exit(1);
    }
    v11 = create_dentry(v15->d_name, &v8, bblock[0] * 64 + bblock + 64);
    if ( v11 )
    {
      fprintf(stderr, "Could not create an entry for %s, skipping it...\n", ptr);
      free(ptr);
    }
    else
    {
      if ( /* *(_DWORD *)bblock << 6 == -0x804A8A0 */ 0 )
        data = create_device();
      else
        data = create_file(ptr, &v8);
      if ( data )
      {
        *((_DWORD *)data + 2047) = bblock[0] * 64 + bblock + 64;
        (*(_DWORD *)bblock)++;
        ++*(_DWORD *)&bblock[4];
        *(_DWORD *)&bblock[8] += *((_DWORD *)data + 2048);
        list_insert_at_tail(&inode_list, data);
      }
      else
      {
        fprintf(stderr, "Could not create an entry for %s, skipping it...\n", ptr);
        free(ptr);
      }
    }
  }
  v12 = init_freemap(*(int32_t *)&bblock[4], *(int32_t *)&bblock[8]);
  *(_DWORD *)&bblock[4] = get_num_inodes(v12);
  *(_DWORD *)&bblock[8] = get_num_datablocks(v12);
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = i->prev )
  {
    buf = i->data;
    *((_DWORD *)buf + 2049) = get_inode_block(v12);
    *(_DWORD *)(*((_DWORD *)buf + 2047) + 36) = *((_DWORD *)buf + 2049);
  }
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = i->prev )
  {
    buf = i->data;
    for ( ptr = 0; (int32_t)ptr < *((_DWORD *)buf + 2048); ++ptr )
      *((_DWORD *)buf + (_DWORD)ptr + 1) = get_data_block(v12);
  }
  if ( write(fd, bblock, 0x1000u) != 4096 )
  {
    perror("write");
    exit(1);
  }
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = i->prev )
  {
    buf = i->data;
    if ( lseek(fd, (*((_DWORD *)buf + 2049) << 12) + 4096, 0) == -1 )
    {
      perror("lseek");
      exit(1);
    }
    if ( write(fd, buf, 0x1000u) != 4096 )
    {
      perror("write");
      exit(1);
    }
  }
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = i->prev )
  {
    buf = i->data;
    for ( data = 0; (int32_t)data < *((_DWORD *)buf + 2048); data = (char *)data + 1 )
    {
      v11 = *((_DWORD *)buf + (_DWORD)data + 1) + get_num_inodes(v12) + 1;
      if ( lseek(fd, v11 << 12, 0) == -1 )
      {
        perror("lseek");
        exit(1);
      }
      if ( write(fd, *((const void **)buf + (_DWORD)data + 1024), 0x1000u) != 4096 )
      {
        perror("write");
        exit(1);
      }
    }
  }
  return 0;
}

//----- (08048F27) --------------------------------------------------------
int32_t create_dentry(const char *src, struct stat *a2, void *dest)
{
  memset(dest, 0, 0x40u);
  if (S_ISREG(a2->st_mode))
  {
    *((_DWORD *)dest + 8) = 2;
  }
  else if (S_ISCHR(a2->st_mode))
  {
    *((_DWORD *)dest + 8) = 0;
  }
  else
  {
    return -1;
  }
  strncpy((char *)dest, src, 0x1Fu);
  *((_BYTE *)dest + 31) = 0;
  *((_DWORD *)dest + 9) = -1;
  return 0;
}

//----- (08048FC4) --------------------------------------------------------
void *create_file(const char *file, struct stat* a2)
{
  int32_t v3; // [sp+Ch] [bp-2Ch]@3
  void *v4; // [sp+14h] [bp-24h]@2
  int32_t j; // [sp+18h] [bp-20h]@8
  int32_t fd; // [sp+20h] [bp-18h]@5
  int32_t i; // [sp+24h] [bp-14h]@5
  void *ptr; // [sp+2Ch] [bp-Ch]@3

  if ( a2->st_size <= 0x3FF000u )
  {
    ptr = malloc(0x2008u);
    memset(ptr, 0, 0x1000u);
    *(_DWORD *)ptr = a2->st_size;
    v3 = a2->st_size / 4096;
    if ( a2->st_size & 0xFFF )
      ++v3;
    *((_DWORD *)ptr + 2048) = v3;
    fd = open(file, O_RDONLY);
    for ( i = 0; i < v3; ++i )
    {
      *((_DWORD *)ptr + i + 1024) = malloc(0x1008u);
      if ( read(fd, *((_DWORD *)ptr + i + 1024), 0x1000u) < 0 )
      {
        for ( j = 0; j < i; ++j )
          free(*((void **)ptr + j + 1024));
        free(ptr);
        ptr = 0;
        break;
      }
    }
    close(fd);
    v4 = ptr;
  }
  else
  {
    v4 = 0;
  }
  return v4;
}

//----- (08049132) --------------------------------------------------------
void *create_device()
{
  void *v0; // ST14_4@1

  v0 = malloc(0x2008u);
  memset(v0, 0, 0x1000u);
  *(_DWORD *)v0 = 0;
  return v0;
}

//----- (08049170) --------------------------------------------------------
void* list_insert_after(inode_root_t *a1, inode_list_t *element, void *data)
{
  inode_list_t *result; // eax@1

  a1->count++;
  result = malloc(sizeof(inode_list_t));
  result->data = data;
  result->prev = element->prev;
  result->next = element;
  element->prev->next = result;
  element->prev = result;
  return result;
}

//----- (080491BF) --------------------------------------------------------
void* list_insert_before(inode_root_t *a1, inode_list_t *element, void *data)
{
  inode_list_t *result; // eax@1

  a1->count++;
  result = malloc(sizeof(inode_list_t));
  result->data = data;
  result->prev = element;
  result->next = element->next;
  element->next->prev = result;
  element->next = result;
  return result;
}

//----- (08049283) --------------------------------------------------------
void* list_insert_at_head(inode_root_t *root, void *data)
{
  return list_insert_after(root, (inode_list_t *)root, data);
}

//----- (080492A4) --------------------------------------------------------
void* list_insert_at_tail(inode_root_t *root, void *data)
{
  return list_insert_before(root, (inode_list_t *)root, data);
}

//----- (080492C8) --------------------------------------------------------
int32_t init_freemap(int32_t a1, int32_t a2)
{
  uint8_t* v2; // eax@1
  uint8_t* v3; // ST0C_4@1
  int32_t v4; // ST10_4@1

  v2 = malloc(0x100);
  v3 = v2;
  *(_DWORD *)v2 = 2 * a1;
  *(_DWORD *)(v2 + 4) = 2 * a2;
  v4 = get_size(v2);
  *(_DWORD *)(v3 + 8) = malloc(v4);
  memset(*(void **)(v3 + 8), 0, v4);
  **(_BYTE **)(v3 + 8) = 1;
  return v3;
}

//----- (08049341) --------------------------------------------------------
int32_t get_inode_block(int32_t a1)
{
  int32_t v2; // [sp+4h] [bp-4h]@1

  do
    v2 = rand() % *(_DWORD *)a1;
  while ( *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + 1) );
  *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + 1) = 1;
  return v2;
}

//----- (08049381) --------------------------------------------------------
int32_t get_data_block(int32_t a1)
{
  int32_t v2; // [sp+4h] [bp-4h]@1

  do
    v2 = rand() % *(_DWORD *)(a1 + 4);
  while ( *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + *(_DWORD *)a1 + 1) );
  *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + *(_DWORD *)a1 + 1) = 1;
  return v2;
}

//----- (080493CC) --------------------------------------------------------
int32_t get_size(int32_t a1)
{
  return *(_DWORD *)a1 + *(_DWORD *)(a1 + 4) + 1;
}

//----- (080493DD) --------------------------------------------------------
int32_t get_num_datablocks(int32_t a1)
{
  return *(_DWORD *)(a1 + 4);
}

//----- (080493E8) --------------------------------------------------------
int32_t get_num_inodes(int32_t a1)
{
  return *(_DWORD *)a1;
}

