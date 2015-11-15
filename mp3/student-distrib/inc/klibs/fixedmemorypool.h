#ifndef _FIXEDMEMORYPOOL_H_
#define _FIXEDMEMORYPOOL_H_

#include <stddef.h>
#include <stdint.h>

namespace util {

template <typename T, size_t capacity>
class MemoryPool
{
private:
    int unusedIndex;
    T memory[capacity];
    T* unusedMemory[capacity];

public:
    MemoryPool()
    {
        unusedIndex = capacity - 1;

        for (size_t i = 0; i < capacity; i++)
        {
            unusedMemory[i] = &memory[i];
        }
    }

    T* get()
    {
        if (unusedIndex < 0) return nullptr;
        return unusedMemory[unusedIndex--];
    }

    void release(T* ptr)
    {
        unusedMemory[++unusedIndex] = ptr;
    }

};

}

#endif
