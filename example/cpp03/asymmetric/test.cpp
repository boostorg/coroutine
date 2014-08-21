#include <boost/coroutine/all.hpp>

typedef boost::coroutines::asymmetric_coroutine< void >::pull_type pull_coro_t;
typedef boost::coroutines::asymmetric_coroutine< void >::push_type push_coro_t;

void foo1( push_coro_t & sink)
{
    for ( int i = 0; i < 10; ++i)
        sink();
}

void foo2( pull_coro_t & source)
{
    while ( source)
        source();
}

void bar()
{
    {
        pull_coro_t source( foo1);
        while ( source) {
            source();
        }
    }
    {
        push_coro_t sink( foo2);
        for ( int i = 0; i < 10; ++i)
            sink();
    }
}

