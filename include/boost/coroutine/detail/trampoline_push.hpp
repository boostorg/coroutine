
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_TRAMPOLINE_PUSH_H
#define BOOST_COROUTINES_DETAIL_TRAMPOLINE_PUSH_H

#include <cstddef>

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
void trampoline_push( intptr_t vp)
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
        BOOST_ASSERT( from->data);

        // create push_coroutine
        typename Self::impl_type b( c.callee_, c.caller_, false, c.preserve_fpu(), from->data);
        Self yield( & b);
        try
        { fn( yield); }
        catch ( forced_unwind const&)
        {}
        catch (...)
        { c.except_ = current_exception(); }
    }

    c.flags_ |= flag_complete;
    param_type to;
    c.callee_->jump(
        * c.caller_,
        reinterpret_cast< intptr_t >( & to),
        c.preserve_fpu() );
    BOOST_ASSERT_MSG( false, "push_coroutine is complete");
}

template< typename Fn, typename Coro, typename Self >
void trampoline_push_void( intptr_t vp)
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
            reinterpret_cast< intptr_t >( & c),
            c.preserve_fpu() );

        // create push_coroutine
        typename Self::impl_type b( c.callee_, c.caller_, false, c.preserve_fpu() );
        Self yield( & b);
        try
        { fn( yield); }
        catch ( forced_unwind const&)
        {}
        catch (...)
        { c.except_ = current_exception(); }
    }

    c.flags_ |= flag_complete;
    param_type to;
    c.callee_->jump(
        * c.caller_,
        reinterpret_cast< intptr_t >( & to),
        c.preserve_fpu() );
    BOOST_ASSERT_MSG( false, "push_coroutine is complete");
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_TRAMPOLINE_PUSH_H
