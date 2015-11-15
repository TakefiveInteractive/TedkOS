#ifndef _X86_STACKER_H
#define _X86_STACKER_H

#include <inc/arch.h>

namespace arch
{
    template <>
    class Stacker<x86>
    {
    private:
        // top <=> the addr. of the LOWEST BYTE
        //  of the last written value.
        uint32_t top;
    public:
        Stacker(void* esp)
        {
            // In machine level a padding of 4 is enough.
            top = (uint32_t)esp - 3;
        }

        Stacker(uint32_t esp)
        {
            // In machine level a padding of 4 is enough.
            top = esp - 3;
        }

        template <typename T>
        Stacker& operator << (const T& value)
        {
            top -= sizeof(T);
            *((T*)top) = value;
            return *this;
        }

        template <typename T>
        Stacker& operator >> (T& value)
        {
            value = *((T*)top);
            top += sizeof(T);
            return *this;
        }

        template <typename T>
        T peek() const
        {
            return *((T*)top);
        }

        template <typename T>
        Stacker& overwrite(const T& value)
        {
            *((T*)top) = value;
            return *this;
        }

        void* getESP() const
        {
            return (void*)top;
        }
    };
}

#endif /* _X86_STACKER_H */
