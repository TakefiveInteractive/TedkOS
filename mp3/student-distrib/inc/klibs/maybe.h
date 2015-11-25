#ifndef _INC_MAYBE_H_
#define _INC_MAYBE_H_

#include <stddef.h>
#include <stdint.h>

namespace maybe_details {

class NothingType {
    uint8_t not_used;
};

}

extern maybe_details::NothingType Nothing;

template <typename T>
class Maybe {
private:
    const T val;
    bool exists;

public:
    Maybe() : val(T{}), exists(false) { }
    Maybe(const T& bla) : val(bla), exists(true) { }
    Maybe(const maybe_details::NothingType nothing) : val(T{}), exists(false) { }

    const T& operator + () const {
        if (!exists)
        {
            // Trigger an exception
            __asm__ __volatile__("int $26;" : : );
            for (;;) { }
            // We'll never reach here
            return val;
        }
        return val;
    }

    const Maybe<T> operator >> (Maybe<T> (*other)()) const {
        if (exists)
        {
            return *this;
        }
        else
        {
            return other();
        }
    }

    bool operator ! () const { return !exists; }
    operator bool() const { return exists; }
};

#endif
