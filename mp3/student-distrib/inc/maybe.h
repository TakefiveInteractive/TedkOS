#ifndef _INC_MAYBE_H_
#define _INC_MAYBE_H_

#include <stddef.h>
#include <stdint.h>

template <typename T>
class Maybe {
private:
    T val;
    bool exists;

public:
    Maybe() : exists(false) { }
    Maybe(T& bla) { val = bla; exists = true; }

    const T& operator ! () const {
        if (!exists)
        {
            // Trigger an exception
            __asm__ __volatile__("int $26;" : : );
            return T();
        }
        return val;
    }

    explicit operator bool() const { return exists; }
};

#endif
