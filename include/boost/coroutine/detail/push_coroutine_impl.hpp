
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_PUSH_COROUTINE_IMPL_H
#define BOOST_COROUTINES_DETAIL_PUSH_COROUTINE_IMPL_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility.hpp>

#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/coroutine/detail/flags.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/detail/trampoline_push.hpp>
#include <boost/coroutine/exceptions.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

struct stack_context;

namespace detail {

template< typename Arg >
class push_coroutine_impl : private noncopyable
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void trampoline_push( intptr_t);

    typedef parameters< Arg >                           param_type;

    int                 flags_;
    exception_ptr       except_;
    coroutine_context   caller_;
    coroutine_context   callee_;

public:
    push_coroutine_impl( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu) :
        flags_( 0),
        except_(),
        caller_( * caller),
        callee_( * callee)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    bool force_unwind() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_force_unwind); }

    bool unwind_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_unwind_stack); }

    bool preserve_fpu() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_preserve_fpu); }

    bool is_complete() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_complete); }

    void unwind_stack() BOOST_NOEXCEPT
    {
        if ( ! is_complete() && force_unwind() )
        {
            flags_ |= flag_unwind_stack;
            param_type to( unwind_t::force_unwind);
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void push( Arg const& arg)
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( const_cast< Arg * >( & arg) );
        param_type * from(
            reinterpret_cast< param_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }

    void push( BOOST_RV_REF( Arg) arg)
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( const_cast< Arg * >( & arg) );
        param_type * from(
            reinterpret_cast< param_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }
};

template< typename Arg >
class push_coroutine_impl< Arg & > : private noncopyable
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void trampoline_push( intptr_t);

    typedef parameters< Arg & >                         param_type;

    int                 flags_;
    exception_ptr       except_;
    coroutine_context   caller_;
    coroutine_context   callee_;

public:
    push_coroutine_impl( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu) :
        flags_( 0),
        except_(),
        caller_( * caller),
        callee_( * callee)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    bool force_unwind() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_force_unwind); }

    bool unwind_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_unwind_stack); }

    bool preserve_fpu() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_preserve_fpu); }

    bool is_complete() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_complete); }

    void unwind_stack() BOOST_NOEXCEPT
    {
        if ( ! is_complete() && force_unwind() )
        {
            flags_ |= flag_unwind_stack;
            param_type to( unwind_t::force_unwind);
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void push( Arg & arg)
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( & arg);
        param_type * from(
            reinterpret_cast< param_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }
};

template<>
class push_coroutine_impl< void > : private noncopyable
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void trampoline_push_void( intptr_t);

    typedef parameters< void >                          param_type;

    int                 flags_;
    exception_ptr       except_;
    coroutine_context   caller_;
    coroutine_context   callee_;

public:
    push_coroutine_impl( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu) :
        flags_( 0),
        except_(),
        caller_( * caller),
        callee_( * callee)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    bool force_unwind() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_force_unwind); }

    bool unwind_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_unwind_stack); }

    bool preserve_fpu() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_preserve_fpu); }

    bool is_complete() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_complete); }

    void unwind_stack() BOOST_NOEXCEPT
    {
        if ( ! is_complete() && force_unwind() )
        {
            flags_ |= flag_unwind_stack;
            param_type to( unwind_t::force_unwind);
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void push()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        param_type * from(
            reinterpret_cast< param_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_PUSH_COROUTINE_IMPL_H
