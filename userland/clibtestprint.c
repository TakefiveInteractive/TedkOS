#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

struct statttttt
{
    dev_t       st_dev;
    ino_t       st_ino;
    mode_t      st_mode;
    nlink_t     t_nlink;
    uid_t       st_uid;
    gid_t       st_gid;
    dev_t       st_rdev;
    off_t       st_size;
};

int main ()
{
    int sz;
    FILE *fp = fopen("frame0.txt", "r");
    printf("Hello, if this ran, the program was correct. Yay!\n");
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    printf("And by the way, ftell says the size of 'frame0.txt' is %d\n", sz);
    struct stat buf;
    int fd = fileno(fp);
    fstat(fd, &buf);
    int size = buf.st_size;
    printf("While fstat says it's %d\n", size);
    return 0;
}

