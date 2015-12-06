#ifndef _SYSCALL_HELPERS_H_
#define _SYSCALL_HELPERS_H_

#include <stdint.h>
#include <boost/type_traits/function_traits.hpp>

namespace syscall {

template<typename F>
F super_cast(uint32_t input)
{
    return reinterpret_cast<F>(input);
}

template<>
int32_t super_cast<int32_t>(uint32_t input)
{
    return static_cast<int32_t>(input);
}

template<unsigned N>
class SystemCallArgN { };

template<>
class SystemCallArgN<0> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr();
    }
};

template<>
class SystemCallArgN<1> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(super_cast<typename boost::function_traits<F>::arg1_type>(p1));
    }
};

template<>
class SystemCallArgN<2> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(
                    super_cast<typename boost::function_traits<F>::arg1_type>(p1),
                    super_cast<typename boost::function_traits<F>::arg2_type>(p2)
                 );
    }
};

template<>
class SystemCallArgN<3> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(
                    super_cast<typename boost::function_traits<F>::arg1_type>(p1),
                    super_cast<typename boost::function_traits<F>::arg2_type>(p2),
                    super_cast<typename boost::function_traits<F>::arg3_type>(p3)
                 );
    }
};

template<typename F>
static int32_t systemCallRunner(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
{
    return SystemCallArgN<boost::function_traits<F>::arity>::run(fptr, p1, p2, p3);
}

}   // namespace syscall

#endif
