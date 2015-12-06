.text
.global getRDTSC

getRDTSC:
    XOR %eax, %eax
    CPUID
    RDTSC
    ret
