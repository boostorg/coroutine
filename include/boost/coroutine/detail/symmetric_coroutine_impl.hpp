
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_IMPL_H
#define BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_IMPL_H

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

template< typename R >
class symmetric_coroutine_impl : private noncopyable
{
private:
    template< typename X >
    friend class symmetric_coroutine_impl;

    template<
        typename X, typename Y, typename Z
    >
    friend void trampoline( intptr_t);

    typedef parameters< R >                           param_type;

    int                     flags_;
    coroutine_context   *   caller_;
    coroutine_context   *   callee_;

    void run_( param_type * to) BOOST_NOEXCEPT
    {
        caller_->jump(
            * callee_,
            reinterpret_cast< intptr_t >( to),
            preserve_fpu() );
    }

    template< typename Other >
    R * yield_to_( Other * other, typename Other::param_type * to)
    {
        other->caller_ = caller_;
        param_type * from(
            reinterpret_cast< param_type * >(
                callee_->jump(
                    * other->callee_,
                    reinterpret_cast< intptr_t >( to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        BOOST_ASSERT( from->data);
        return from->data;
    }

public:
    symmetric_coroutine_impl( coroutine_context * caller,
                              coroutine_context * callee,
                              bool unwind, bool preserve_fpu) BOOST_NOEXCEPT :
        flags_( 0),
        caller_( caller),
        callee_( callee)
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
            caller_->jump(
                * callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void run( R const& r) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( const_cast< R * >( & r) );
        run_( & to);
    }

    void run( BOOST_RV_REF( R) r) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( const_cast< R * >( & r) );
        run_( & to);
    }

    R * yield()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        param_type * from(
            reinterpret_cast< param_type * >(
                callee_->jump(
                    * caller_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        BOOST_ASSERT( from->data);
        return from->data;
    }

    template< typename X >
    R * yield_to( symmetric_coroutine_impl< X > * other, X const& x)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X >::param_type to( const_cast< X * >( & x) );
        return yield_to_( other, & to);
    }

    template< typename X >
    R * yield_to( symmetric_coroutine_impl< X & > * other, X const& x)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X & >::param_type to( const_cast< X * >( & x) );
        return yield_to_( other, & to);
    }

    template< typename X >
    R * yield_to( symmetric_coroutine_impl< X > * other)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X >::param_type to;
        return yield_to_( other, & to);
    }
};

template< typename R >
class symmetric_coroutine_impl< R & > : private noncopyable
{
private:
    template< typename X >
    friend class symmetric_coroutine_impl;

    template<
        typename X, typename Y, typename Z
    >
    friend void trampoline( intptr_t);

    typedef parameters< R & >                         param_type;

    int                     flags_;
    coroutine_context   *   caller_;
    coroutine_context   *   callee_;

    void run_( param_type * to) BOOST_NOEXCEPT
    {
        caller_->jump(
            * callee_,
            reinterpret_cast< intptr_t >( to),
            preserve_fpu() );
    }

    template< typename Other >
    R * yield_to_( Other * other, typename Other::param_type * to)
    {
        other->caller_ = caller_;
        param_type * from(
            reinterpret_cast< param_type * >(
                callee_->jump(
                    * other->callee_,
                    reinterpret_cast< intptr_t >( to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        BOOST_ASSERT( from->data);
        return from->data;
    }

public:
    symmetric_coroutine_impl( coroutine_context * caller,
                              coroutine_context * callee,
                              bool unwind, bool preserve_fpu) BOOST_NOEXCEPT :
        flags_( 0),
        caller_( caller),
        callee_( callee)
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
            caller_->jump(
                * callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void run( R & arg) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( & arg);
        run_( & to);
    }

    R * yield()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        param_type * from(
            reinterpret_cast< param_type * >(
                callee_->jump(
                    * caller_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        BOOST_ASSERT( from->data);
        return from->data;
    }

    template< typename X >
    R * yield_to( symmetric_coroutine_impl< X > * other, X const& x)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X >::param_type to( const_cast< X * >( & x) );
        return yield_to_( other, & to);
    }

    template< typename X >
    R * yield_to( symmetric_coroutine_impl< X & > * other, X const& x)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X & >::param_type to( const_cast< X * >( & x) );
        return yield_to_( other, & to);
    }

    template< typename X >
    R * yield_to( symmetric_coroutine_impl< X > * other)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X >::param_type to;
        return yield_to_( other, & to);
    }
};

template<>
class symmetric_coroutine_impl< void > : private noncopyable
{
private:
    template< typename X >
    friend class symmetric_coroutine_impl;

    template<
        typename X, typename Y, typename Z
    >
    friend void trampoline_void( intptr_t);

    typedef parameters< void >                          param_type;

    int                     flags_;
    coroutine_context   *   caller_;
    coroutine_context   *   callee_;

    template< typename Other >
    void yield_to_( Other * other, typename Other::param_type * to)
    {
        other->caller_ = caller_;
        param_type * from(
            reinterpret_cast< param_type * >(
                callee_->jump(
                    * other->callee_,
                    reinterpret_cast< intptr_t >( to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
    }

public:
    symmetric_coroutine_impl( coroutine_context * caller,
                              coroutine_context * callee,
                              bool unwind, bool preserve_fpu) BOOST_NOEXCEPT :
        flags_( 0),
        caller_( caller),
        callee_( callee)
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
            caller_->jump(
                * callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void run() BOOST_NOEXCEPT
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        caller_->jump(
            * callee_,
            reinterpret_cast< intptr_t >( & to),
            preserve_fpu() );
    }

    template< typename X >
    void yield_to( symmetric_coroutine_impl< X > * other, X x)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X >::param_type to( & x);
        yield_to_( other, & to);
    }

    void yield() BOOST_NOEXCEPT
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        param_type * from(
            reinterpret_cast< param_type * >(
                callee_->jump(
                    * caller_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
    }

    template< typename X >
    void yield_to( symmetric_coroutine_impl< X > * other, X const& x)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X >::param_type to( const_cast< X * >( & x) );
        yield_to_( other, & to);
    }

    template< typename X >
    void yield_to( symmetric_coroutine_impl< X & > * other, X const& x)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X & >::param_type to( const_cast< X * >( & x) );
        yield_to_( other, & to);
    }

    template< typename X >
    void yield_to( symmetric_coroutine_impl< X > * other)
    {
        BOOST_ASSERT( ! is_complete() );
        BOOST_ASSERT( ! other->is_complete() );

        typename symmetric_coroutine_impl< X >::param_type to;
        yield_to_( other, & to);
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_IMPL_H
