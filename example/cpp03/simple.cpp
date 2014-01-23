
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>

typedef boost::coroutines::coroutine< void >::pull_type pull_coro_t;
typedef boost::coroutines::coroutine< void >::push_type push_coro_t;

void runit( push_coro_t & sink)
{
    std::cout << "started! ";
    for ( int i = 0; i < 10; ++i)
    {
        sink();
    }
}

int main( int argc, char * argv[])
{
    {
        pull_coro_t source( runit);
        while ( source) {
            source();
        }
    }

    std::cout << "\nDone" << std::endl;

    return EXIT_SUCCESS;
}
