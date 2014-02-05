
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_TRAMPOLINE_H
#define BOOST_COROUTINES_DETAIL_TRAMPOLINE_H

#include <cstddef>
#include <exception>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/move/move.hpp>

#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/detail/flags.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/detail/setup.hpp>
#include <boost/coroutine/detail/setup.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/coroutine/flags.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {
namespace detail {

template< typename Fn, typename Coro, typename Self >
void trampoline( intptr_t vp)
{
    typedef typename Coro::param_type   param_type;

    BOOST_ASSERT( vp);

    setup< Fn > * from(
        reinterpret_cast< setup< Fn > * >( vp) );

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    Fn fn( forward< Fn >( from->fn) );
#else
    Fn fn( move( from->fn) );
#endif
    Coro c( from->caller, from->callee,
            stack_unwind == from->attr.do_unwind,
            fpu_preserved == from->attr.preserve_fpu);
    from = 0;

    {
        param_type * from(
            reinterpret_cast< param_type * >(
                c.callee_->jump(
                    * c.caller_,
                    reinterpret_cast< intptr_t >(  & c),
                    c.preserve_fpu() ) ) );

        // create yield_type
        Self yield( & c, from->data);
        try
        { fn( yield); }
        catch ( forced_unwind const&)
        {}
        catch (...)
        { std::terminate(); }
    }

    c.flags_ |= flag_complete;
    c.callee_->jump( * c.caller_, 0, c.preserve_fpu() );
    BOOST_ASSERT_MSG( false, "coroutine is complete");
}

template< typename Fn, typename Coro, typename Self >
void trampoline_void( intptr_t vp)
{
    typedef typename Coro::param_type   param_type;

    BOOST_ASSERT( vp);

    setup< Fn > * from(
        reinterpret_cast< setup< Fn > * >( vp) );

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    Fn fn( forward< Fn >( from->fn) );
#else
    Fn fn( move( from->fn) );
#endif
    Coro c( from->caller, from->callee,
            stack_unwind == from->attr.do_unwind,
            fpu_preserved == from->attr.preserve_fpu);
    from = 0;

    {
        c.callee_->jump(
            * c.caller_,
            reinterpret_cast< intptr_t >(  & c),
            c.preserve_fpu() );

        // create yield_type
        Self yield( & c);
        try
        { fn( yield); }
        catch ( forced_unwind const&)
        {}
        catch (...)
        { std::terminate(); }
    }

    c.flags_ |= flag_complete;
    param_type to;
    c.callee_->jump(
        * c.caller_,
        reinterpret_cast< intptr_t >( & to),
        c.preserve_fpu() );
    BOOST_ASSERT_MSG( false, "coroutine is complete");
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_TRAMPOLINE_H
