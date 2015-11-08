#ifndef _INC_BITSET_H_
#define _INC_BITSET_H_

#include <climits>

namespace util {

template<size_t n>
class BitSet {
  private:
    uint32_t array[n];
    static constexpr size_t IntBits = CHAR_BIT * sizeof(uint32_t);

  public:
    BitSet() : array() { }

    void set(size_t k)
    {
        if (k >= n) return;
        array[k / IntBits] |= (1U << (k % IntBits));
    }

    void clear(size_t)
    {
        if (k >= n) return;
        array[k / IntBits] &= ~(1U << (k % IntBits));
    }

    void flip(size_t)
    {
        if (k >= n) return;
        array[k / IntBits] ^= (1U << (k % IntBits));
    }

    bool test(size_t) const
    {
        if (k >= n) return false;
        return ((array[k / IntBits] & (1U << (k % IntBits))) != 0);
    }
};

}

#endif
