#ifndef _FIXEDMEMORYPOOL_H_
#define _FIXEDMEMORYPOOL_H_

#include <stddef.h>
#include <stdint.h>

namespace util {

template <typename T, unsigned int capacity>
class MemoryPool
{
public:
    MemoryPool()
    {
        unusedIndex = capacity - 1;

        for (int i = 0; i < capacity; i++)
        {
            unusedMemory[i] = &memory[i];
        }
    }

    T* get(size_t size)
    {
        if (unusedIndex < 0) return nullptr;
        return unusedMemory[unusedIndex--];
    }

    void release(T* ptr)
    {
        unusedMemory[++unusedIndex] = ptr;
    }

private:
    int uusedIndex;
    T memory[capacity];
    T* unusedMemory[capacity];
};

}

#endif
