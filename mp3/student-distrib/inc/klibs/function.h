#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <stddef.h>
#include <inc/klibs/fixedmemorypool.h>
using namespace util;

template<typename Result,typename ...Args>
struct abstract_function
{
    virtual Result operator()(Args... args)=0;
    virtual ~abstract_function() = default;
};

template<typename Func,typename Result,typename ...Args>
class concrete_function: public abstract_function<Result,Args...>
{
    Func f;
public:
    concrete_function(const Func &x)
        : f(x)
    {}
    Result operator()(Args... args) override
    {
        return f(args...);
    }

    // custom new and delete
    void *operator new(size_t s, MemoryPool<uint8_t[64], 3>& hp)   { return reinterpret_cast<concrete_function*>(hp.get()); }
};

template<typename Func>
struct func_filter
{
    typedef Func type;
};
template<typename Result,typename ...Args>
struct func_filter<Result(Args...)>
{
    typedef Result (*type)(Args...);
};

template<typename signature>
class function;

template<typename Result,typename ...Args>
class function<Result(Args...)>
{
    abstract_function<Result,Args...> *f;
    // Size-agnostic pool. 64 should be enough for most functions
    MemoryPool<uint8_t[64], 3> pool;

public:
    function() : f(nullptr) { }

    template<typename Func> function(const Func &x)
    {
        f = new (pool) concrete_function<typename func_filter<Func>::type,Result,Args...>(x);
    }

    Result operator()(Args... args)
    {
        if(f)
            return (*f)(args...);
        else
            return Result{};
    }

    ~function()
    {
        f->~abstract_function<Result,Args...>();
        pool.release(reinterpret_cast<uint8_t(*)[64]>(f));
    }
};

#endif
