
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_CALL_H
#define BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_CALL_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/explicit_operator_bool.hpp>

#include <boost/coroutine/attributes.hpp>
#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/detail/setup.hpp>
#include <boost/coroutine/detail/symmetric_coroutine_impl.hpp>
#include <boost/coroutine/detail/symmetric_coroutine_yield.hpp>
#include <boost/coroutine/detail/trampoline.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {
namespace detail {

template< typename Arg, typename StackAllocator >
class symmetric_coroutine_call
{
private:
    template< typename X >
    friend class symmetric_coroutine_yield;

    typedef symmetric_coroutine_impl< Arg >   impl_type;
    typedef parameters< Arg >                 param_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine_call)

    struct dummy {};

    impl_type       *   impl_;
    StackAllocator      stack_alloc_;
    stack_context       stack_ctx_;
    coroutine_context   caller_;
    coroutine_context   callee_;

public:
    typedef Arg                                value_type;
    typedef symmetric_coroutine_yield< Arg >   yield_type;

    symmetric_coroutine_call() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
    typedef void ( * coroutine_fn)( yield_type &);

    explicit symmetric_coroutine_call( coroutine_fn fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< coroutine_fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    explicit symmetric_coroutine_call( coroutine_fn fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< coroutine_fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
# endif
    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#else
    template< typename Fn >
    explicit symmetric_coroutine_call( Fn fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( Fn fn, attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#endif

    ~symmetric_coroutine_call() BOOST_NOEXCEPT
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    symmetric_coroutine_call( BOOST_RV_REF( symmetric_coroutine_call) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    symmetric_coroutine_call & operator=( BOOST_RV_REF( symmetric_coroutine_call) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine_call tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete() || impl_->is_running(); }

    void swap( symmetric_coroutine_call & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    symmetric_coroutine_call & operator()( Arg arg) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( * this);

        impl_->run( arg);
        return * this;
    }
};

template< typename Arg, typename StackAllocator >
class symmetric_coroutine_call< Arg &, StackAllocator >
{
private:
    template< typename X >
    friend class symmetric_coroutine_yield;

    typedef symmetric_coroutine_impl< Arg & >     impl_type;
    typedef parameters< Arg & >                   param_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine_call)

    struct dummy {};

    impl_type       *   impl_;
    StackAllocator      stack_alloc_;
    stack_context       stack_ctx_;
    coroutine_context   caller_;
    coroutine_context   callee_;

public:
    typedef Arg                                    value_type;
    typedef symmetric_coroutine_yield< Arg & >     yield_type;

    symmetric_coroutine_call() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
    typedef void ( * coroutine_fn)( yield_type &);

    explicit symmetric_coroutine_call( coroutine_fn fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< coroutine_fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    explicit symmetric_coroutine_call( coroutine_fn fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< coroutine_fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
# endif
    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#else
    template< typename Fn >
    explicit symmetric_coroutine_call( Fn fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( Fn fn, attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#endif

    ~symmetric_coroutine_call() BOOST_NOEXCEPT
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    symmetric_coroutine_call( BOOST_RV_REF( symmetric_coroutine_call) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    symmetric_coroutine_call & operator=( BOOST_RV_REF( symmetric_coroutine_call) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine_call tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete() || impl_->is_running(); }

    void swap( symmetric_coroutine_call & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    symmetric_coroutine_call & operator()( Arg & arg) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( * this);

        impl_->run( arg);
        return * this;
    }
};

template< typename StackAllocator >
class symmetric_coroutine_call< void, StackAllocator >
{
private:
    template< typename X >
    friend class symmetric_coroutine_yield;

    typedef symmetric_coroutine_impl< void >        impl_type;
    typedef parameters< void >                      param_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine_call)

    struct dummy {};

    impl_type       *   impl_;
    StackAllocator      stack_alloc_;
    stack_context       stack_ctx_;
    coroutine_context   caller_;
    coroutine_context   callee_;

public:
    typedef void                                     value_type;
    typedef symmetric_coroutine_yield< void >        yield_type;

    symmetric_coroutine_call() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
    typedef void ( * coroutine_fn)( yield_type &);

    explicit symmetric_coroutine_call( coroutine_fn fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< coroutine_fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    explicit symmetric_coroutine_call( coroutine_fn fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< coroutine_fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
# endif
    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#else
    template< typename Fn >
    explicit symmetric_coroutine_call( Fn fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( Fn fn, attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine_call( BOOST_RV_REF( Fn) fn,
                                       attributes const& attr,
                                       StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = coroutine_context(
            trampoline_void< Fn, impl_type, yield_type >,
            & stack_ctx_);
        setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#endif

    ~symmetric_coroutine_call() BOOST_NOEXCEPT
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    symmetric_coroutine_call( BOOST_RV_REF( symmetric_coroutine_call) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    symmetric_coroutine_call & operator=( BOOST_RV_REF( symmetric_coroutine_call) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine_call tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete() || impl_->is_running(); }

    void swap( symmetric_coroutine_call & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    symmetric_coroutine_call & operator()() BOOST_NOEXCEPT
    {
        BOOST_ASSERT( * this);

        impl_->run();
        return * this;
    }
};

template< typename Arg, typename StackAllocator >
void swap( symmetric_coroutine_call< Arg, StackAllocator > & l,
           symmetric_coroutine_call< Arg, StackAllocator > & r)
{ l.swap( r); }

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_CALL_H
