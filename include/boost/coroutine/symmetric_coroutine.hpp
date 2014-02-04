
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_SYMMETRIC_COROUTINE_H
#define BOOST_COROUTINES_SYMMETRIC_COROUTINE_H

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
#include <boost/coroutine/detail/symmetric_coroutine_self.hpp>
#include <boost/coroutine/detail/trampoline.hpp>
#include <boost/coroutine/stack_allocator.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

template< typename T, typename StackAllocator = standard_stack_allocator >
class symmetric_coroutine
{
private:
    template< typename X >
    friend class detail::symmetric_coroutine_self;

    typedef detail::symmetric_coroutine_impl< T >   impl_type;
    typedef detail::parameters< T >                 param_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine)

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

public:
    typedef T                                       value_type;
    typedef detail::symmetric_coroutine_self< T >   self_type;

    symmetric_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( self_type &);

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< coroutine_fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< coroutine_fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
# endif
    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#else
    template< typename Fn >
    explicit symmetric_coroutine( Fn fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                     dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( Fn fn, attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#endif

    ~symmetric_coroutine() BOOST_NOEXCEPT
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    symmetric_coroutine( BOOST_RV_REF( symmetric_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    symmetric_coroutine & operator=( BOOST_RV_REF( symmetric_coroutine) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( symmetric_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    symmetric_coroutine & operator()( T t) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( * this);

        impl_->run( t);
        return * this;
    }

    symmetric_coroutine & operator()( BOOST_RV_REF( T) t) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( * this);

        impl_->run( forward< T >( t) );
        return * this;
    }
};

template< typename T, typename StackAllocator >
class symmetric_coroutine< T &, StackAllocator >
{
private:
    template< typename X >
    friend class detail::symmetric_coroutine_self;

    typedef detail::symmetric_coroutine_impl< T & >     impl_type;
    typedef detail::parameters< T & >                   param_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine)

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

public:
    typedef T                                           value_type;
    typedef detail::symmetric_coroutine_self< T & >     self_type;

    symmetric_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( self_type &);

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< coroutine_fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< coroutine_fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
# endif
    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#else
    template< typename Fn >
    explicit symmetric_coroutine( Fn fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                     dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( Fn fn, attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#endif

    ~symmetric_coroutine() BOOST_NOEXCEPT
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    symmetric_coroutine( BOOST_RV_REF( symmetric_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    symmetric_coroutine & operator=( BOOST_RV_REF( symmetric_coroutine) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( symmetric_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    symmetric_coroutine & operator()( T & t) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( * this);

        impl_->run( t);
        return * this;
    }
};

template< typename StackAllocator >
class symmetric_coroutine< void, StackAllocator >
{
private:
    template< typename X >
    friend class detail::symmetric_coroutine_self;

    typedef detail::symmetric_coroutine_impl< void >        impl_type;
    typedef detail::parameters< void >                      param_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine)

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

public:
    typedef void                                            value_type;
    typedef detail::symmetric_coroutine_self< void >        self_type;

    symmetric_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( self_type &);

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< coroutine_fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< coroutine_fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
# endif
    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#else
    template< typename Fn >
    explicit symmetric_coroutine( Fn fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                     dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( Fn fn, attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_void< Fn, impl_type, self_type >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
    }
#endif

    ~symmetric_coroutine() BOOST_NOEXCEPT
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    symmetric_coroutine( BOOST_RV_REF( symmetric_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    symmetric_coroutine & operator=( BOOST_RV_REF( symmetric_coroutine) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( symmetric_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    symmetric_coroutine & operator()() BOOST_NOEXCEPT
    {
        BOOST_ASSERT( * this);

        impl_->run();
        return * this;
    }
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_SYMMETRIC_COROUTINE_H
