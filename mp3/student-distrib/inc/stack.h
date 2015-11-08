#ifndef _INC_STACK_H_
#define _INC_STACK_H_

#include <inc/maybe.h>

namespace util {

template<typename T, size_t Num>
class Stack {
    private:
        T arr[Num];
        size_t idx;

    public:
        Stack() : arr(), idx(0) { }

        bool empty()
        {
            return (idx == 0);
        }

        bool full()
        {
            return (idx == Num);
        }

        T pop()
        {
            if (idx == 0)
            {
                return T();
            }
            else
            {
                idx--;
                return arr[idx];
            }
        }

        void push(T val)
        {
            if (idx >= Num)
            {
                return;
            }
            else
            {
                arr[idx] = val;
                idx++;
            }
        }

        Maybe<void *> first(Maybe<void *> (*fn) (T))
        {
            for (size_t i = 0; i < idx; i++)
            {
                auto x = fn(arr[idx]);
                if (x) return x;
            }
            return Maybe<void *>();
        }

        bool firstTrue(void *a, bool (*fn) (T, void*))
        {
            for (size_t i = 0; i < idx; i++)
            {
                auto x = fn(arr[idx], a);
                if (x) return x;
            }
            return false;
        }
};

}

#endif
