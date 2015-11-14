#ifndef _UTIL_PREFIX_TREE_H_
#define _UTIL_PREFIX_TREE_H_

#include <stddef.h>
#include <stdint.h>

#include <inc/klibs/fixedmemorypool.h>
#include <inc/klibs/lib.h>

namespace util {

template<typename T>
class PrefixTreeNode {
public:
    T value = T();
    PrefixTreeNode<T> *children[256] = {};
    bool hasValue = false;

    // custom new and delete
    void *operator new(size_t s, MemoryPool<PrefixTreeNode<T>, 100>& hp)   { return hp.get(); }
    void operator delete(void *dp, MemoryPool<PrefixTreeNode<T>, 100>& hp)    { hp.release(dp); }

    void *operator new[](size_t s, MemoryPool<PrefixTreeNode<T>, 100>& hp) { return hp.get(); }
    void operator delete[](void *dp, MemoryPool<PrefixTreeNode<T>, 100>& hp)  { hp.release(dp); }

};

template<typename T>
struct ValAndLength {
    T val;
    size_t len;
};

template<typename T>
class PrefixTree {
private:
    size_t count;
    PrefixTreeNode<T> *root;
    MemoryPool<PrefixTreeNode<T>, 100> pool;
public:
    PrefixTree()
    {
        count = 0;
        root = new (pool) PrefixTreeNode<T>();
    }

    void insert(const char *key, T val)
    {
        int length = strlen(key);
        int index;

        count++;
        auto pCrawl = root;

        for (int level = 0; level < length; level++)
        {
            index = key[level];
            if(!pCrawl->children[index])
            {
                pCrawl->children[index] = new (pool) PrefixTreeNode<T>();
            }
            pCrawl = pCrawl->children[index];
        }

        // mark last node as leaf
        pCrawl->hasValue = true;
        pCrawl->value = val;
    }

    ValAndLength<T> search(const char *key)
    {
        size_t level;
        size_t length = strlen(key);
        int index;
        size_t lastLevel = 0;

        auto pCrawl = root;
        auto lastVal = T();

        for (level = 0; level < length; level++)
        {
            index = key[level];

            if(!pCrawl->children[index])
            {
                break;
            }

            pCrawl = pCrawl->children[index];

            if (pCrawl->hasValue)
            {
                lastVal = pCrawl->value;
                lastLevel = level;
            }
        }

        return { .val = lastVal, .len = lastLevel + 1 };
    }
};

}

#endif

