#ifndef _INC_MAYBE_H_
#define _INC_MAYBE_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/x86/err_handler.h>

namespace maybe_details {

class NothingType {
    uint8_t not_used;
};

}

extern maybe_details::NothingType Nothing;

template <typename T>
union MaybeUnion
{
    uint8_t _nothing;
    T v;
    MaybeUnion() : _nothing(0) {}
    ~MaybeUnion() {}
};

template <typename T>
class Maybe {
private:
    MaybeUnion<T> val;
    bool exists;

public:
    Maybe() : exists(false) { }
    Maybe(const T& bla) : exists(true) { val.v = bla; }
    Maybe(const Maybe<T>& other) : exists(other.exists)
    {
        if (exists)
        {
            val.v = other.val.v;
        }
    }
    ~Maybe() { if (exists) (&val.v)->~T(); }
    Maybe(const maybe_details::NothingType nothing) : exists(false) { }

    Maybe& operator = (const Maybe& other)
    {
        if(exists && (!other.exists))
            (&val.v)->~T();
        exists = other.exists;
        if (exists)
        {
            val.v = other.val.v;
        }
        return *this;
    }

    const T& operator + () const {
        if (!exists)
        {
            // Trigger an exception
            trigger_exception<26>();
            // We'll never reach here
            for (;;) { }
            return val.v;
        }
        return val.v;
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
