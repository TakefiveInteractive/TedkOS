.text
.global getRDTSC

getRDTSC:
    XOR %eax, %eax
    pushal
    CPUID
    RDTSC
    popal
    ret
