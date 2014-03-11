
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>

void coro(boost::coroutines::coroutine<void>::pull_type& c)
{
      std::printf("in coro!\n");
}

int main()
{
    {
      boost::coroutines::coroutine<void>::push_type p(coro);
        std::printf("after construction\n");
    }
    return EXIT_SUCCESS;
}
