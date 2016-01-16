#ifndef _INC_BITSET_H_
#define _INC_BITSET_H_

#include <inc/klibs/maybe.h>

namespace util {

template<size_t n>
class BitSet {
  private:
    uint32_t array[n];
    static constexpr size_t IntBits = 8 * sizeof(uint32_t);

  public:
    BitSet() : array() { }

    void set(size_t k)
    {
        if (k >= n) return;
        array[k / IntBits] |= (1U << (k % IntBits));
    }

    void clear(size_t k)
    {
        if (k >= n) return;
        array[k / IntBits] &= ~(1U << (k % IntBits));
    }

    void flip(size_t k)
    {
        if (k >= n) return;
        array[k / IntBits] ^= (1U << (k % IntBits));
    }

    bool test(size_t k) const
    {
        if (k >= n) return false;
        return ((array[k / IntBits] & (1U << (k % IntBits))) != 0);
    }

    Maybe<size_t> findConsZeros(size_t num) const
    {
        size_t currMax = 0;
        size_t startOfMax = 0;
        bool foundSomething = false;
        for (size_t i = 0; i < n; i++)
        {
            if (test(num) == false)
            {
                currMax++;
                if (!foundSomething) startOfMax = i;
                foundSomething = true;
                if (currMax >= num) return Maybe<size_t>(startOfMax);
            }
            else
            {
                currMax = 0;
                foundSomething = false;
            }
        }
        return Maybe<size_t>();
    }
};

}

#endif
