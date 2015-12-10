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
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
    {
        return super_cast<int32_t>(fptr());
    }
};

template<>
class SystemCallArgN<1> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
    {
        return reinterpret_cast<int32_t>(fptr(
                    super_cast<typename boost::function_traits<F>::arg1_type>(p1)));
    }
};

template<>
class SystemCallArgN<2> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
    {
        return reinterpret_cast<int32_t>(fptr(
                    super_cast<typename boost::function_traits<F>::arg1_type>(p1),
                    super_cast<typename boost::function_traits<F>::arg2_type>(p2)
                 ));
    }
};

template<>
class SystemCallArgN<3> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
    {
        return reinterpret_cast<int32_t>(fptr(
                    super_cast<typename boost::function_traits<F>::arg1_type>(p1),
                    super_cast<typename boost::function_traits<F>::arg2_type>(p2),
                    super_cast<typename boost::function_traits<F>::arg3_type>(p3)
                 ));
    }
};

template<>
class SystemCallArgN<4> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
    {
        return reinterpret_cast<int32_t>(fptr(
                    super_cast<typename boost::function_traits<F>::arg1_type>(p1),
                    super_cast<typename boost::function_traits<F>::arg2_type>(p2),
                    super_cast<typename boost::function_traits<F>::arg3_type>(p3),
                    super_cast<typename boost::function_traits<F>::arg3_type>(p4)
                 ));
    }
};

template<typename F>
static int32_t systemCallRunner(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    return SystemCallArgN<boost::function_traits<F>::arity>::run(fptr, p1, p2, p3, p4);
}

}   // namespace syscall

#endif
