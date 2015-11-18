#ifndef _LPHASHTABLE_H_
#define _LPHASHTABLE_H_

#include <stddef.h>
#include <stdint.h>

namespace util {

template<typename From, typename To>
class HashEntry {
    private:
        From key;
        To value;
        bool _isSet;
    public:

        HashEntry()
        {
            key = From();
            value = To();
            _isSet = false;
        }

        From getKey()
        {
            return key;
        }

        To getValue()
        {
            return value;
        }

        bool isSet()
        {
            return _isSet;
        }

        void set(From key, To val)
        {
            this->key = key;
            this->value = val;
            this->_isSet = true;
        }
};

template<uint32_t size, typename From, typename To, typename HashFn>
class LinearProbingHashTable {
    private:
        HashEntry<From, To> table[size] = {};
    public:
        LinearProbingHashTable() {}

        To get(From key, bool& found)
        {
            uint32_t hash = HashFn::hash(key) % size;
            while (table[hash].isSet() && table[hash].getKey() != key)
                hash = (hash + 1) % size;
            if (!table[hash].isSet())
            {
                found = false;
                return To();
            }
            else
            {
                found = true;
                return table[hash].getValue();
            }
        }

        void put(From key, To value)
        {
            uint32_t hash = HashFn::hash(key) % size;
            while (table[hash].isSet() && table[hash].getKey() != key)
                hash = (hash + 1) % size;
            table[hash].set(key, value);
        }
};

}

#endif
