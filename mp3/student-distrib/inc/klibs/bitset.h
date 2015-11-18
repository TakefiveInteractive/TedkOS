#ifndef _INC_BITSET_H_
#define _INC_BITSET_H_

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
};

}

#endif
