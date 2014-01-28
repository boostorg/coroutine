
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>

struct X
{
    int i;

    X( int i_) :
        i( i_)
    {}
};

typedef boost::coroutines::coroutine< void >::pull_type pull_coro_t;
typedef boost::coroutines::coroutine< void >::push_type push_coro_t;

void fn1( push_coro_t & sink)
{
    for ( int i = 0; i < 10; ++i)
    {
        sink();
    }
}

void fn2( pull_coro_t & source)
{
    while ( source) {
        std::cout << "i = " << std::endl;
        source();
    }
}

int main( int argc, char * argv[])
{
    {
        pull_coro_t source( fn1);
        while ( source) {
//          X * x = source.get();
            std::cout << "i = " << std::endl;
            source();
        }
    }
    {
        push_coro_t sink( fn2);
        for ( int i = 0; i < 10; ++i)
        {
            sink();
        }
    }
    std::cout << "Done" << std::endl;

    return EXIT_SUCCESS;
}
