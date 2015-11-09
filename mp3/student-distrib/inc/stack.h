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

        bool empty() const
        {
            return (idx == 0);
        }

        bool full() const
        {
            return (idx == Num);
        }

        size_t size() const
        {
            return idx;
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

        T drop(size_t killIdx)
        {
            T mem = arr[killIdx];
            // Copy everything over
            for (size_t i = killIdx; i < idx - 1; i++)
            {
                arr[i] = arr[i + 1];
            }
            idx--;
            return mem;
        }

        T get(size_t x) const { return arr[x]; }

        template<typename R> Maybe<R> first(Maybe<R> (*fn) (T));

        bool firstTrue(void *a, size_t &in_idx, bool (*fn) (T, void*))
        {
            for (size_t i = 0; i < idx; i++)
            {
                auto x = fn(arr[i], a);
                if (x)
                {
                    in_idx = i;
                    return x;
                }
            }
            return false;
        }
};

template<typename T, size_t Num>
template<typename R>
Maybe<R> Stack<T, Num>::first(Maybe<R> (*fn) (T))
{
    for (size_t i = 0; i < idx; i++)
    {
        auto x = fn(arr[i]);
        if (x) return x;
    }
    return Maybe<R>();
}

}

#endif
