How to compile
==============

```sh
/Volumes/ece391cc/cross/bin/i686-elf-gcc -Wall -nostdlib --freestanding -Iclib/i686-tedkos/include -I. -Isyscall syscall/ece391syscall.o -Lclib/i686-tedkos/lib -lc -lg -lm -lnosys clibtestprint.c -o clibtestprint
```
