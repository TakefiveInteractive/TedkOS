#ifndef _INC_STACK_H_
#define _INC_STACK_H_

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
};

}

#endif
