
namespace palloc {

template <size_t MaxMemory>
PhysPageManager<MaxMemory>::PhysPageManager()
{
    // 0 and 1 are pages used by kernel initialization
    isPhysAddrUsed.set(0);
    isPhysAddrUsed.set(1);

    for(uint16_t i = 30; i < 0x3f; i++)
        freePhysAddr.push(i);
}

template <size_t MaxMemory>
uint16_t PhysPageManager<MaxMemory>::allocPage(int8_t isCommonPage)
{
    if(freePhysAddr.empty())
        return 0xffff;
    uint16_t top = freePhysAddr.pop();
    isPhysAddrUsed.set(top);
    if(isCommonPage)
        this->isCommonPage.set(top);
    return top;
}

template <size_t MaxMemory>
void PhysPageManager<MaxMemory>::freePage(uint16_t pageIndex)
{
    if(isPhysAddrUsed.test(pageIndex))
        return;
    isPhysAddrUsed.clear(pageIndex);
    isCommonPage.clear(pageIndex);
    freePhysAddr.push(pageIndex);
    return;
}


template <size_t startAddr, size_t MaxSize>
VirtualMemRegion<startAddr,MaxSize>::VirtualMemRegion()
{
    for(size_t i = 0; i < MaxNumElements; i++)
        freeVirtAddr.push(i);
}

template <size_t startAddr, size_t MaxSize>
void* VirtualMemRegion<startAddr,MaxSize>::allocPage(int8_t isCommonPage)
{
    if(freeVirtAddr.empty())
        return (void*)0xffff;
    uint16_t top = freeVirtAddr.pop();
    isVirtAddrUsed.set(top);
    if(isCommonPage)
        this->isCommonPage.set(top);
    return (void*)(top << 22);
}

template <size_t startAddr, size_t MaxSize>
void VirtualMemRegion<startAddr,MaxSize>::freePage(void* virtAddr)
{
    uint16_t pageIndex = ((uint32_t)virtAddr >> 22);
    if(isVirtAddrUsed.test(pageIndex))
        return;
    isVirtAddrUsed.clear(pageIndex);
    isCommonPage.clear(pageIndex);
    freeVirtAddr.push(pageIndex);
    return;
}

}

