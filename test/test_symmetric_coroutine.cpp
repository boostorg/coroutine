
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cstdio>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/move/move.hpp>
#include <boost/range.hpp>
#include <boost/ref.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>

#include <boost/coroutine/all.hpp>

namespace coro = boost::coroutines;

bool value1 = false;
int value2 = 0;
std::string value3;

coro::symmetric_coroutine< void > * term_coro = 0;

struct X
{
    int i;

    X() :
        i( 0)
    {}

    X( int i_) :
        i( i_)
    {}
};

X * p = 0;

struct Y
{
    Y() 
    { value2 = 7; }

    ~Y() 
    { value2 = 0; }
};

class copyable
{
public:
    bool    state;

    copyable() :
        state( false)
    {}

    copyable( int) :
        state( true)
    {}

    void operator()( coro::symmetric_coroutine< int >::self_type &)
    { value1 = state; }
};

class moveable
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE( moveable);

public:
    bool    state;

    moveable() :
        state( false)
    {}

    moveable( int) :
        state( true)
    {}

    moveable( BOOST_RV_REF( moveable) other) :
        state( false)
    { std::swap( state, other.state); }

    moveable & operator=( BOOST_RV_REF( moveable) other)
    {
        if ( this == & other) return * this;
        moveable tmp( boost::move( other) );
        std::swap( state, tmp.state);
        return * this;
    }

    void operator()( coro::symmetric_coroutine< int >::self_type &)
    { value1 = state; }
};

void empty( coro::symmetric_coroutine< void >::self_type &) {}

void f2( coro::symmetric_coroutine< void >::self_type &)
{ ++value2; }

void f3( coro::symmetric_coroutine< X >::self_type & self)
{ if ( self) value2 = self.get().i; }

void f4( coro::symmetric_coroutine< X& >::self_type & self)
{
    if ( self)
    {
       X & x = self.get();
       p = & x;
    }
}

void f5( coro::symmetric_coroutine< X* >::self_type & self)
{ if ( self) p = self.get(); }

void f6( coro::symmetric_coroutine< void >::self_type & self)
{
    Y y;
    self( *term_coro);
}

void f7( coro::symmetric_coroutine< int >::self_type & self)
{
    if ( self) value2 = self.get();
    self( *term_coro);
    if ( self) value2 = self.get();
}

void f8( coro::symmetric_coroutine< int >::self_type & self)
{
    try
    {
        int i = self.get();
        (void)i;
    }
    catch ( coro::invalid_result const&)
    {
        value1 = true; 
    }
}

template< typename E >
void f9( coro::symmetric_coroutine< void >::self_type &, E const& e)
{ throw e; }

void f10( coro::symmetric_coroutine< int >::self_type & self,
          coro::symmetric_coroutine< int > & other)
{
    int i = self.get();
    self( other, i);
    value2 = self.get();
}

void f101( coro::symmetric_coroutine< int >::self_type & self)
{ value2 = self.get(); }

void f11( coro::symmetric_coroutine< void >::self_type & self,
          coro::symmetric_coroutine< void > & other)
{
    self( other);
    value2 = 7;
}

void f111( coro::symmetric_coroutine< void >::self_type &)
{ value2 = 3; }

void f12( coro::symmetric_coroutine< X& >::self_type & self,
          coro::symmetric_coroutine< X& > & other)
{
    self( other, self.get());
    p = & self.get();
}

void f121( coro::symmetric_coroutine< X& >::self_type & self)
{ p = & self.get(); }

void f13( coro::symmetric_coroutine< int >::self_type & self,
          coro::symmetric_coroutine< int > & other)
{
    self( other);
    value2 = self.get();
}

void f131( coro::symmetric_coroutine< int >::self_type & self)
{ if ( self) value2 = self.get(); }

void f14( coro::symmetric_coroutine< int >::self_type & self,
          coro::symmetric_coroutine< std::string > & other)
{
    std::string str( boost::lexical_cast< std::string >( self.get() ) );
    self( other, str);
    value2 = self.get();
}

void f141( coro::symmetric_coroutine< std::string >::self_type & self)
{ if ( self) value3 = self.get(); }

void f15( coro::symmetric_coroutine< int >::self_type & self,
          int offset,
          coro::symmetric_coroutine< int > & other)
{
    int x = self.get();
    value2 += x + offset;
    self( other, x);
    x = self.get();
    value2 += x + offset;
    self( other, x);
}

void f151( coro::symmetric_coroutine< int >::self_type & self,
          int offset)
{
    int x = self.get();
    value2 += x + offset;
    self();
    x = self.get();
    value2 += x + offset;
}

void test_move()
{
    {
        coro::symmetric_coroutine< void > coro1;
        coro::symmetric_coroutine< void > coro2( empty);
        BOOST_CHECK( ! coro1);
        BOOST_CHECK( coro2);
        coro1 = boost::move( coro2);
        BOOST_CHECK( coro1);
        BOOST_CHECK( ! coro2);
    }

    {
        value1 = false;
        copyable cp( 3);
        BOOST_CHECK( cp.state);
        BOOST_CHECK( ! value1);
        coro::symmetric_coroutine< int > coro( cp);
        coro();
        BOOST_CHECK( cp.state);
        BOOST_CHECK( value1);
    }

    {
        value1 = false;
        moveable mv( 7);
        BOOST_CHECK( mv.state);
        BOOST_CHECK( ! value1);
        coro::symmetric_coroutine< int > coro( boost::move( mv) );
        coro();
        BOOST_CHECK( ! mv.state);
        BOOST_CHECK( value1);
    }
}

void test_complete()
{
    value2 = 0;

    coro::symmetric_coroutine< void > coro( f2);
    BOOST_CHECK( coro);
    coro();
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int)1, value2);
}

void test_yield()
{
    value2 = 0;

    coro::symmetric_coroutine< int > coro3(
        boost::bind( f151, _1, 3) );
    BOOST_CHECK( coro3);
    coro::symmetric_coroutine< int > coro2(
        boost::bind( f15, _1, 2, boost::ref( coro3) ) );
    BOOST_CHECK( coro2);
    coro::symmetric_coroutine< int > coro1(
        boost::bind( f15, _1, 1, boost::ref( coro2) ) );
    BOOST_CHECK( coro1);

    BOOST_CHECK_EQUAL( ( int)0, value2);
    coro1( 1);
    BOOST_CHECK( coro3);
    BOOST_CHECK( coro2);
    BOOST_CHECK( coro1);
    BOOST_CHECK_EQUAL( ( int)9, value2);
    coro1( 2);
    BOOST_CHECK( ! coro3);
    BOOST_CHECK( coro2);
    BOOST_CHECK( coro1);
    BOOST_CHECK_EQUAL( ( int)21, value2);
}

void test_pass_value()
{
    value2 = 0;

    X x(7);
    BOOST_CHECK_EQUAL( ( int)7, x.i);
    BOOST_CHECK_EQUAL( 0, value2);
    coro::symmetric_coroutine< X > coro( f3);
    BOOST_CHECK( coro);
    coro();
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int)7, x.i);
    BOOST_CHECK_EQUAL( 0, value2);

    BOOST_CHECK_EQUAL( ( int)7, x.i);
    BOOST_CHECK_EQUAL( 0, value2);
    coro = coro::symmetric_coroutine< X >( f3);
    BOOST_CHECK( coro);
    coro(7);
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int)7, x.i);
    BOOST_CHECK_EQUAL( 7, value2);
}

void test_pass_reference()
{
    p = 0;

    X x;
    coro::symmetric_coroutine< X& > coro( f4);
    BOOST_CHECK( coro);
    coro();
    BOOST_CHECK( ! coro);
    BOOST_CHECK( 0 == p);

    coro = coro::symmetric_coroutine< X& >( f4);
    BOOST_CHECK( coro);
    coro( x);
    BOOST_CHECK( ! coro);
    BOOST_CHECK( p == & x);
}

void test_pass_pointer()
{
    p = 0;

    X x;
    coro::symmetric_coroutine< X* > coro( f5);
    BOOST_CHECK( coro);
    coro();
    BOOST_CHECK( ! coro);
    BOOST_CHECK( 0 == p);

    coro = coro::symmetric_coroutine< X* >( f5);
    BOOST_CHECK( coro);
    coro( & x);
    BOOST_CHECK( ! coro);
    BOOST_CHECK( p == & x);
}

void test_unwind()
{
    value2 = 0;
    {
        coro::symmetric_coroutine< void > coro( f6);
        coro::symmetric_coroutine< void > coro_e( empty);
        BOOST_CHECK( coro);
        BOOST_CHECK( coro_e);
        term_coro = & coro_e;
        BOOST_CHECK_EQUAL( ( int) 0, value2);
        coro();
        BOOST_CHECK( coro);
        BOOST_CHECK_EQUAL( ( int) 7, value2);
    }
    BOOST_CHECK_EQUAL( ( int) 0, value2);
}

void test_no_unwind()
{
    value2 = 0;
    {
        coro::symmetric_coroutine< void > coro( f6,
            coro::attributes(
                coro::stack_allocator::default_stacksize(),
                coro::no_stack_unwind) );
        coro::symmetric_coroutine< void > coro_e( empty);
        BOOST_CHECK( coro);
        BOOST_CHECK( coro_e);
        term_coro = & coro_e;
        BOOST_CHECK_EQUAL( ( int) 0, value2);
        coro();
        BOOST_CHECK( coro);
        BOOST_CHECK_EQUAL( ( int) 7, value2);
    }
    BOOST_CHECK_EQUAL( ( int) 7, value2);
}

void test_termination()
{
    value2 = 0;

    coro::symmetric_coroutine< int > coro( f7);
    coro::symmetric_coroutine< void > coro_e( empty);
    BOOST_CHECK( coro);
    BOOST_CHECK( coro_e);
    term_coro = & coro_e;
    BOOST_CHECK_EQUAL( ( int) 0, value2);
    coro(3);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 3, value2);
    coro(7);
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int) 7, value2);
}

void test_invalid_arg()
{
    value1 = false;

    coro::symmetric_coroutine< int > coro( f8);
    BOOST_CHECK( coro);
    BOOST_CHECK( ! value1);
    coro();
    BOOST_CHECK( ! coro);
    BOOST_CHECK( value1);
}

void test_yield_to_void()
{
    value2 = 0;

    coro::symmetric_coroutine< void > coro_other( f111);
    coro::symmetric_coroutine< void > coro( boost::bind( f11, _1, boost::ref( coro_other) ) );
    BOOST_CHECK( coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 0, value2);
    coro();
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 3, value2);
    coro();
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int) 7, value2);
}

void test_yield_to_int()
{
    value2 = 0;

    coro::symmetric_coroutine< int > coro_other( f101);
    coro::symmetric_coroutine< int > coro( boost::bind( f10, _1, boost::ref( coro_other) ) );
    BOOST_CHECK( coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 0, value2);
    coro(3);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 3, value2);
    coro(7);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int) 7, value2);
}

void test_yield_to_ref()
{
    p = 0;

    coro::symmetric_coroutine< X& > coro_other( f121);
    coro::symmetric_coroutine< X& > coro( boost::bind( f12, _1, boost::ref( coro_other) ) );
    BOOST_CHECK( coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK( 0 == p);
    X x1(3);
    coro( x1);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( p->i, x1.i);
    BOOST_CHECK( p == & x1);
    X x2(7);
    coro(x2);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( p->i, x2.i);
    BOOST_CHECK( p == & x2);
}

void test_yield_to_nothing()
{
    value2 = 0;

    coro::symmetric_coroutine< int > coro_other( f131);
    coro::symmetric_coroutine< int > coro( boost::bind( f13, _1, boost::ref( coro_other) ) );
    BOOST_CHECK( coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 0, value2);
    coro(3);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 0, value2);
    coro(7);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int) 7, value2);
}

void test_yield_to_different()
{
    value2 = 0;
    value3 = "";

    coro::symmetric_coroutine< std::string > coro_other( f141);
    coro::symmetric_coroutine< int > coro( boost::bind( f14, _1, boost::ref( coro_other) ) );
    BOOST_CHECK( coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( ( int) 0, value2);
    BOOST_CHECK( value3.empty() );
    coro(3);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( coro);
    BOOST_CHECK_EQUAL( "3", value3);
    coro(7);
    BOOST_CHECK( ! coro_other);
    BOOST_CHECK( ! coro);
    BOOST_CHECK_EQUAL( ( int) 7, value2);
}

boost::unit_test::test_suite * init_unit_test_suite( int, char* [])
{
    boost::unit_test::test_suite * test =
        BOOST_TEST_SUITE("Boost.coroutine: symmetric coroutine test suite");

    test->add( BOOST_TEST_CASE( & test_move) );
    test->add( BOOST_TEST_CASE( & test_complete) );
    test->add( BOOST_TEST_CASE( & test_yield) );
    test->add( BOOST_TEST_CASE( & test_pass_value) );
    test->add( BOOST_TEST_CASE( & test_pass_reference) );
    test->add( BOOST_TEST_CASE( & test_pass_pointer) );
    test->add( BOOST_TEST_CASE( & test_invalid_arg) );
    test->add( BOOST_TEST_CASE( & test_termination) );
    test->add( BOOST_TEST_CASE( & test_unwind) );
    test->add( BOOST_TEST_CASE( & test_no_unwind) );
    test->add( BOOST_TEST_CASE( & test_yield_to_void) );
    test->add( BOOST_TEST_CASE( & test_yield_to_int) );
    test->add( BOOST_TEST_CASE( & test_yield_to_ref) );
    test->add( BOOST_TEST_CASE( & test_yield_to_nothing) );
    test->add( BOOST_TEST_CASE( & test_yield_to_different) );

    return test;
}
