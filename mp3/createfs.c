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

inode_root_t inode_list = { .head = &inode_list, .tail = &inode_list, .data = 0, .count = 0 };

int main(int a1, char * a2[]);
signed int create_dentry(const char *src, int a2, void *dest);
void *create_file(const char *file, int a2);
void *create_device();
int list_insert_after(inode_root_t *a1, inode_list_t *element, void *data);
int list_insert_before(inode_root_t *a1, inode_list_t *element, void *data);
int list_remove(int, void *ptr); // idb
int list_remove_tail(inode_root_t *root);
int list_remove_head(inode_root_t *root);
int list_insert_at_head(inode_root_t *root, void *data);
int list_insert_at_tail(inode_root_t *root, void *data);
int init_freemap(int a1, int a2);
int get_inode_block(int a1);
int get_data_block(int a1);
int get_size(int a1);
int get_num_datablocks(int a1);
int get_num_inodes(int a1);

//----- (080488B4) --------------------------------------------------------
int main(int a1, char *argv[])
{
  size_t v3; // eax@5
  size_t v4; // ebx@18
  size_t v5; // ebx@18
  size_t v6; // eax@18
  int v8; // [sp+10h] [bp-98h]@18
  char *ptr; // [sp+74h] [bp-34h]@18
  void *data; // [sp+78h] [bp-30h]@24
  int v11; // [sp+7Ch] [bp-2Ch]@21
  int v12; // [sp+80h] [bp-28h]@33
  int i; // [sp+84h] [bp-24h]@33
  void *buf; // [sp+88h] [bp-20h]@35
  struct dirent *v15; // [sp+8Ch] [bp-1Ch]@17
  DIR *dirp; // [sp+90h] [bp-18h]@12
  int fd; // [sp+94h] [bp-14h]@8
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
  fd = open(file, 194, 33188);
  if ( fd == -1 )
  {
    fprintf(stderr, "open: Output file %s exists, do you want to overwrite?\n(y/n): ", file);
    if ( getchar() == 110 )
      exit(1);
    fd = open(file, 2, 33188);
  }
  src = *(char **)(argv[1]);
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
    if ( stat(ptr, (struct stat *)&v8) )
    {
      perror("stat");
      exit(1);
    }
    v11 = create_dentry(v15->d_name, (int)&v8, (void *)((*(_DWORD *)bblock << 6) + 134523008));
    if ( v11 )
    {
      fprintf(stderr, "Could not create an entry for %s, skipping it...\n", ptr);
      free(ptr);
    }
    else
    {
      if ( *(_DWORD *)bblock << 6 == -134523040 )
        data = create_device();
      else
        data = create_file(ptr, (int)&v8);
      if ( data )
      {
        *((_DWORD *)data + 2047) = ((*(_DWORD *)bblock)++ << 6) + 134523008;
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
  v12 = init_freemap(*(int *)&bblock[4], *(int *)&bblock[8]);
  *(_DWORD *)&bblock[4] = get_num_inodes(v12);
  *(_DWORD *)&bblock[8] = get_num_datablocks(v12);
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = *(_DWORD *)i )
  {
    buf = *(void **)(i + 8);
    *((_DWORD *)buf + 2049) = get_inode_block(v12);
    *(_DWORD *)(*((_DWORD *)buf + 2047) + 36) = *((_DWORD *)buf + 2049);
  }
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = *(_DWORD *)i )
  {
    buf = *(void **)(i + 8);
    for ( ptr = 0; (signed int)ptr < *((_DWORD *)buf + 2048); ++ptr )
      *((_DWORD *)buf + (_DWORD)ptr + 1) = get_data_block(v12);
  }
  if ( write(fd, bblock, 0x1000u) != 4096 )
  {
    perror("write");
    exit(1);
  }
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = *(_DWORD *)i )
  {
    buf = *(void **)(i + 8);
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
  for ( i = inode_list.head; (inode_root_t *)i != &inode_list; i = *(_DWORD *)i )
  {
    buf = *(void **)(i + 8);
    for ( data = 0; (signed int)data < *((_DWORD *)buf + 2048); data = (char *)data + 1 )
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
signed int create_dentry(const char *src, int a2, void *dest)
{
  memset(dest, 0, 0x40u);
  if ( (*(_DWORD *)(a2 + 16) & 0xF000) == 32768 )
  {
    *((_DWORD *)dest + 8) = 2;
LABEL_6:
    strncpy((char *)dest, src, 0x1Fu);
    *((_BYTE *)dest + 31) = 0;
    *((_DWORD *)dest + 9) = -1;
    return 0;
  }
  if ( (*(_DWORD *)(a2 + 16) & 0xF000) == 8192 )
  {
    *((_DWORD *)dest + 8) = 0;
    goto LABEL_6;
  }
  return -1;
}

//----- (08048FC4) --------------------------------------------------------
void *create_file(const char *file, int a2)
{
  int v3; // [sp+Ch] [bp-2Ch]@3
  void *v4; // [sp+14h] [bp-24h]@2
  int j; // [sp+18h] [bp-20h]@8
  int fd; // [sp+20h] [bp-18h]@5
  int i; // [sp+24h] [bp-14h]@5
  void *ptr; // [sp+2Ch] [bp-Ch]@3

  if ( *(_DWORD *)(a2 + 44) <= 0x3FF000u )
  {
    ptr = malloc(0x2008u);
    memset(ptr, 0, 0x1000u);
    *(_DWORD *)ptr = *(_DWORD *)(a2 + 44);
    v3 = *(_DWORD *)(a2 + 44) / 4096;
    if ( *(_DWORD *)(a2 + 44) & 0xFFF )
      ++v3;
    *((_DWORD *)ptr + 2048) = v3;
    fd = open(file, 0);
    for ( i = 0; i < v3; ++i )
    {
      *((_DWORD *)ptr + i + 1024) = malloc(0x1008u);
      if ( read(fd, *((void **)ptr + i + 1024), 0x1000u) < 0 )
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
int list_insert_after(inode_root_t *a1, inode_list_t *element, void *data)
{
  int result; // eax@1

  ++a1->count;
  result = (int)malloc(sizeof(inode_list_t));
  *(_DWORD *)(result + 8) = data;
  *(_DWORD *)result = element->prev;
  *(_DWORD *)(result + 4) = element;
  *(_DWORD *)(element->prev + 4) = result;
  element->prev = result;
  return result;
}

//----- (080491BF) --------------------------------------------------------
int list_insert_before(inode_root_t *a1, inode_list_t *element, void *data)
{
  int result; // eax@1

  ++a1->count;
  result = (int)malloc(sizeof(inode_list_t));
  *(_DWORD *)(result + 8) = data;
  *(_DWORD *)result = element;
  *(_DWORD *)(result + 4) = element->next;
  *(_DWORD *)element->next = result;
  element->next = result;
  return result;
}

//----- (08049210) --------------------------------------------------------
int list_remove(int a1, void *ptr)
{
  int v2; // ST04_4@1

  v2 = *((_DWORD *)ptr + 2);
  *(_DWORD *)(*(_DWORD *)ptr + 4) = *((_DWORD *)ptr + 1);
  **((_DWORD **)ptr + 1) = *(_DWORD *)ptr;
  free(ptr);
  return v2;
}

//----- (0804924A) --------------------------------------------------------
int list_remove_tail(inode_root_t *root)
{
  return list_remove((int)root, (void *)root->tail);
}

//----- (08049267) --------------------------------------------------------
int list_remove_head(inode_root_t *root)
{
  return list_remove((int)root, (void *)root->head);
}

//----- (08049283) --------------------------------------------------------
int list_insert_at_head(inode_root_t *root, void *data)
{
  return list_insert_after(root, (inode_list_t *)root, data);
}

//----- (080492A4) --------------------------------------------------------
int list_insert_at_tail(inode_root_t *root, void *data)
{
  return list_insert_before(root, (inode_list_t *)root, data);
}

//----- (080492C8) --------------------------------------------------------
int init_freemap(int a1, int a2)
{
  int v2; // eax@1
  int v3; // ST0C_4@1
  int v4; // ST10_4@1

  v2 = (int)malloc(sizeof(inode_list_t));
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
int get_inode_block(int a1)
{
  int v2; // [sp+4h] [bp-4h]@1

  do
    v2 = rand() % *(_DWORD *)a1;
  while ( *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + 1) );
  *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + 1) = 1;
  return v2;
}

//----- (08049381) --------------------------------------------------------
int get_data_block(int a1)
{
  int v2; // [sp+4h] [bp-4h]@1

  do
    v2 = rand() % *(_DWORD *)(a1 + 4);
  while ( *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + *(_DWORD *)a1 + 1) );
  *(_BYTE *)(*(_DWORD *)(a1 + 8) + v2 + *(_DWORD *)a1 + 1) = 1;
  return v2;
}

//----- (080493CC) --------------------------------------------------------
int get_size(int a1)
{
  return *(_DWORD *)a1 + *(_DWORD *)(a1 + 4) + 1;
}

//----- (080493DD) --------------------------------------------------------
int get_num_datablocks(int a1)
{
  return *(_DWORD *)(a1 + 4);
}

//----- (080493E8) --------------------------------------------------------
int get_num_inodes(int a1)
{
  return *(_DWORD *)a1;
}

