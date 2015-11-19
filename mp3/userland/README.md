How to compile
==============

```sh
/Volumes/ece391cc/cross/bin/i686-elf-gcc -Wall -nostdlib --freestanding -Iclib/i686-tedkos/include -I. -Isyscall clibtestprint.c syscall/ece391syscall.o -Lclib/i686-tedkos/lib -lc -lg -lm -lnosys -o clibtestprint
```
