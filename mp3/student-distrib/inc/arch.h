#ifndef _ARCH_H
#define _ARCH_H

#include <stdint.h>
#include <stddef.h>

/**
 *
 * This file defines some interfaces implemented for
 *   any CPU Architecture.
 *
 */

namespace arch
{
    typedef enum 
    {
        x86
    } CPUArchTypes;

    // Fact:
    //      The pushed structure MUST use the correct ENDIAN.
    //      YOU MUST ALSO MAKE SURE gcc DOES NOT insert
    //          unwanted SPACING to align things.
    //      Little endian:
    //          The first item in struct will be poped first.
    template <CPUArchTypes type>
    class Stacker
    {
    public:

        template <typename T>
        Stacker& operator << (const T& value);

        template <typename T>
        Stacker& operator >> (T& value);

        template <typename T>
        T peek() const;

        template <typename T>
        Stacker& overwrite(const T& value);

        void* getESP() const;
    };
}

#endif /* _ARCH_H */
