
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/coroutine/all.hpp>

typedef boost::coroutines::symmetric_coroutine< void >  coro_t;

int main( int argc, char * argv[])
{
    coro_t * other1 = 0;
    coro_t * other2 = 0;

    coro_t coro1(
        [&]( coro_t::self_type & self) {
            std::cout << "foo1" << std::endl;
            self( * other2);
            std::cout << "foo2" << std::endl;
            self( * other2);
            std::cout << "foo3" << std::endl;
        });
    coro_t coro2(
        [&]( coro_t::self_type & self) {
            std::cout << "bar1" << std::endl;
            self( * other1);
            std::cout << "bar2" << std::endl;
            self( * other1);
            std::cout << "bar3" << std::endl;
        });

    other1 = & coro1;
    other2 = & coro2;

    coro1();

    std::cout << "Done" << std::endl;

    return EXIT_SUCCESS;
}