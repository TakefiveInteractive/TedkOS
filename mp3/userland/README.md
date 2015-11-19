How to compile
==============

```sh
/Volumes/ece391cc/cross/bin/i686-elf-gcc -Wall -nostdlib -Iclib/i686-tedkos/include -I. -Isyscall clibtestprint.c ../fish/ece391syscall.o -Lclib/i686-tedkos/lib -lc -lg -lm -lnosys -o clibtestprint
```
