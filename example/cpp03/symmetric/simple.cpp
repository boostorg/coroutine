
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>

struct Y
{
    Y()
    { std::cout << "Y()" <<std::endl; }

    ~Y()
    { std::cout << "~Y()" <<std::endl; }
};

struct X
{
    int     i;

    X( int i_) :
        i( i_)
    {}
};

typedef boost::coroutines::symmetric_coroutine< X&>  coro1_t;
typedef boost::coroutines::symmetric_coroutine< X&>  coro2_t;

coro1_t  * c1;
coro2_t  * c2;

void fn1( coro1_t::self_type & self)
{
    Y y;
    X x( 0);
    std::cout << "fn1: start" << std::endl;

    if ( self) x = self.get();
    std::cout << "fn1: i == " << x.i << std::endl;
    self( * c2, X(x.i + 1));

    if ( self) x = self.get();
    std::cout << "fn1: i == " << x.i << std::endl;
    self( * c2, X(x.i + 1));
    std::cout << "fn1: finish" << std::endl;
}

void fn2( coro2_t::self_type & self)
{
    std::cout << "fn2: start" << std::endl;
    X x( 0);
    if ( self) x = self.get();
    std::cout << "fn2: i == " << x.i << std::endl;
    self( * c1, X(7));
    if ( self) x = self.get();
    std::cout << "fn2: i == " << x.i << std::endl;
    std::cout << "fn2: finish" << std::endl;
}

int main( int argc, char * argv[])
{
    coro1_t c1_( fn1);
    coro2_t c2_( fn2);
    c1 = & c1_;
    c2 = & c2_;
    X x(3);
    c1_( x);
    std::cout << "Done" << std::endl;

    return EXIT_SUCCESS;
}
