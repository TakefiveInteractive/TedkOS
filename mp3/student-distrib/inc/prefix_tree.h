#ifndef _UTIL_PREFIX_TREE_H_
#define _UTIL_PREFIX_TREE_H_

#include <stddef.h>
#include <stdint.h>

#include <inc/fixedmemorypool.h>

namespace util {

template<typename T>
class PrefixTreeNode {
    T value;
    PrefixTreeNode *children[256];
};

template<typename T>
class PrefixTree {

};

}

#endif

