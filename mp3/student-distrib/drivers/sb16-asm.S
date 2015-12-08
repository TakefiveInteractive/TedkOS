.text
.global getRDTSC

getRDTSC:
    XOR %eax, %eax
    XOR %edx, %edx
    pushl %ebx
    pushl %ecx
    CPUID
    RDTSC
    popl %ecx
    popl %ebx
    ret
