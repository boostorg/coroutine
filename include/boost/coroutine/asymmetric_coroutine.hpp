
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_ASYMMETRIC_COROUTINE_H
#define BOOST_COROUTINES_ASYMMETRIC_COROUTINE_H

#include <cstddef>
#include <iterator>
#include <memory>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/range.hpp>
#include <boost/throw_exception.hpp>
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
#include <boost/coroutine/detail/trampoline_pull.hpp>
#include <boost/coroutine/detail/trampoline_push.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/coroutine/stack_allocator.hpp>
#include <boost/coroutine/detail/pull_coroutine_impl.hpp>
#include <boost/coroutine/detail/push_coroutine_impl.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

template< typename R, typename StackAllocator >
class pull_coroutine;

template< typename Arg, typename StackAllocator >
class push_coroutine
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void detail::trampoline_pull( intptr_t);

    typedef detail::push_coroutine_impl< Arg >  impl_type;
    typedef detail::parameters< Arg >           param_type;

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( push_coroutine)

    push_coroutine( impl_type * impl) :
        impl_( impl),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { BOOST_ASSERT( impl_); }

public:
    push_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( pull_coroutine< Arg, StackAllocator > &);

    explicit push_coroutine( coroutine_fn fn,
                             attributes const& attr = attributes() );

    explicit push_coroutine( coroutine_fn fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc);
# endif
    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes() );

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc);
#else
    template< typename Fn >
    explicit push_coroutine( Fn fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( Fn fn, attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                 dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, push_coroutine >,
                                 dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, push_coroutine >,
                                 dummy*
                             >::type = 0);
#endif

    ~push_coroutine()
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    push_coroutine( BOOST_RV_REF( push_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    push_coroutine & operator=( BOOST_RV_REF( push_coroutine) other) BOOST_NOEXCEPT
    {
        push_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( push_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    push_coroutine & operator()( Arg arg)
    {
        BOOST_ASSERT( * this);

        impl_->push( arg);
        return * this;
    }

    class iterator : public std::iterator< std::output_iterator_tag, void, void, void, void >
    {
    private:
       push_coroutine< Arg, StackAllocator >    *   c_;

    public:
        iterator() :
           c_( 0)
        {}

        explicit iterator( push_coroutine< Arg, StackAllocator > * c) :
            c_( c)
        {}

        iterator & operator=( Arg a)
        {
            BOOST_ASSERT( c_);
            if ( ! ( * c_)( a) ) c_ = 0;
            return * this;
        }

        bool operator==( iterator const& other)
        { return other.c_ == c_; }

        bool operator!=( iterator const& other)
        { return other.c_ != c_; }

        iterator & operator*()
        { return * this; }

        iterator & operator++()
        { return * this; }
    };

    struct const_iterator;
};

template< typename Arg, typename StackAllocator >
class push_coroutine< Arg &, StackAllocator >
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void detail::trampoline_pull( intptr_t);

    typedef detail::push_coroutine_impl< Arg & >    impl_type;
    typedef detail::parameters< Arg & >             param_type;

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( push_coroutine)

    push_coroutine( impl_type * impl) :
        impl_( impl),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { BOOST_ASSERT( impl_); }

public:
    push_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( pull_coroutine< Arg &, StackAllocator > &);

    explicit push_coroutine( coroutine_fn fn,
                             attributes const& attr = attributes() );

    explicit push_coroutine( coroutine_fn fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc);
# endif
    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes() );

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc);
#else
    template< typename Fn >
    explicit push_coroutine( Fn fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                 dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( Fn fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                 dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, push_coroutine >,
                                 dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, push_coroutine >,
                                 dummy*
                             >::type = 0);
#endif

    ~push_coroutine()
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    push_coroutine( BOOST_RV_REF( push_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    push_coroutine & operator=( BOOST_RV_REF( push_coroutine) other) BOOST_NOEXCEPT
    {
        push_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( push_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    push_coroutine & operator()( Arg & arg)
    {
        BOOST_ASSERT( * this);

        impl_->push( arg);
        return * this;
    }

    class iterator : public std::iterator< std::output_iterator_tag, void, void, void, void >
    {
    private:
       push_coroutine< Arg &, StackAllocator >  *   c_;

    public:
        iterator() :
           c_( 0)
        {}

        explicit iterator( push_coroutine< Arg &, StackAllocator > * c) :
            c_( c)
        {}

        iterator & operator=( Arg & a)
        {
            BOOST_ASSERT( c_);
            if ( ! ( * c_)( a) ) c_ = 0;
            return * this;
        }

        bool operator==( iterator const& other)
        { return other.c_ == c_; }

        bool operator!=( iterator const& other)
        { return other.c_ != c_; }

        iterator & operator*()
        { return * this; }

        iterator & operator++()
        { return * this; }
    };

    struct const_iterator;
};

template< typename StackAllocator >
class push_coroutine< void, StackAllocator >
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void detail::trampoline_pull_void( intptr_t);

    typedef detail::push_coroutine_impl< void >     impl_type;
    typedef detail::parameters< void >              param_type;

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( push_coroutine)

    push_coroutine( impl_type * impl) :
        impl_( impl),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { BOOST_ASSERT( impl_); }

public:
    push_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( pull_coroutine< void, StackAllocator > &);

    explicit push_coroutine( coroutine_fn fn,
                             attributes const& attr = attributes() );

    explicit push_coroutine( coroutine_fn fn, attributes const& attr,
                             StackAllocator const& stack_alloc);
# endif
    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes() );

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc);
#else
    template< typename Fn >
    explicit push_coroutine( Fn fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                 dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( Fn fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, push_coroutine >,
                                 dummy*
                             >::type = 0);

    template< typename Fn >
    explicit push_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, push_coroutine >,
                                 dummy*
                             >::type = 0);
#endif

    ~push_coroutine()
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    push_coroutine( BOOST_RV_REF( push_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    push_coroutine & operator=( BOOST_RV_REF( push_coroutine) other) BOOST_NOEXCEPT
    {
        push_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( push_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    push_coroutine & operator()()
    {
        BOOST_ASSERT( * this);

        impl_->push();
        return * this;
    }

    struct iterator;
    struct const_iterator;
};



template< typename R, typename StackAllocator >
class pull_coroutine
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void detail::trampoline_push( intptr_t);

    typedef detail::pull_coroutine_impl< R >    impl_type;
    typedef detail::parameters< R >             param_type;

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( pull_coroutine)

    pull_coroutine( impl_type * impl) :
        impl_( impl),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { BOOST_ASSERT( impl_); }

public:
    pull_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( push_coroutine< R, StackAllocator > &);

    explicit pull_coroutine( coroutine_fn fn,
                             attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_pull< coroutine_fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    explicit pull_coroutine( coroutine_fn fn,
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
            detail::trampoline_pull< coroutine_fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
# endif
    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
#else
    template< typename Fn >
    explicit pull_coroutine( Fn fn,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( Fn fn,
                             attributes const& attr,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, pull_coroutine >,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, pull_coroutine >,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
#endif

    ~pull_coroutine()
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    pull_coroutine( BOOST_RV_REF( pull_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    pull_coroutine & operator=( BOOST_RV_REF( pull_coroutine) other) BOOST_NOEXCEPT
    {
        pull_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( pull_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    pull_coroutine & operator()()
    {
        BOOST_ASSERT( * this);

        impl_->pull();
        return * this;
    }

    R get() const
    {
        BOOST_ASSERT( 0 != impl_);

        return impl_->get();
    }

    class iterator : public std::iterator< std::input_iterator_tag, typename remove_reference< R >::type >
    {
    private:
        pull_coroutine< R, StackAllocator > *   c_;
        R                                   *   val_;

        void fetch_()
        {
            BOOST_ASSERT( c_);

            if ( ! ( * c_) )
            {
                c_ = 0;
                val_ = 0;
                return;
            }
            val_ = c_->impl_->get_pointer();
        }

        void increment_()
        {
            BOOST_ASSERT( c_);
            BOOST_ASSERT( * c_);

            ( * c_)();
            fetch_();
        }

    public:
        typedef typename iterator::pointer      pointer_t;
        typedef typename iterator::reference    reference_t;

        iterator() :
            c_( 0), val_( 0)
        {}

        explicit iterator( pull_coroutine< R, StackAllocator > * c) :
            c_( c), val_( 0)
        { fetch_(); }

        iterator( iterator const& other) :
            c_( other.c_), val_( other.val_)
        {}

        iterator & operator=( iterator const& other)
        {
            if ( this == & other) return * this;
            c_ = other.c_;
            val_ = other.val_;
            return * this;
        }

        bool operator==( iterator const& other)
        { return other.c_ == c_ && other.val_ == val_; }

        bool operator!=( iterator const& other)
        { return other.c_ != c_ || other.val_ != val_; }

        iterator & operator++()
        {
            increment_();
            return * this;
        }

        iterator operator++( int)
        {
            iterator tmp( * this);
            ++*this;
            return tmp;
        }

        reference_t operator*() const
        {
            if ( ! val_)
                boost::throw_exception(
                    invalid_result() );
            return * val_;
        }

        pointer_t operator->() const
        {
            if ( ! val_)
                boost::throw_exception(
                    invalid_result() );
            return val_;
        }
    };

    friend class iterator;

    struct const_iterator;
};

template< typename R, typename StackAllocator >
class pull_coroutine< R &, StackAllocator >
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void detail::trampoline_push( intptr_t);

    typedef detail::pull_coroutine_impl< R & >  impl_type;
    typedef detail::parameters< R & >           param_type;

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( pull_coroutine)

    pull_coroutine( impl_type * impl) :
        impl_( impl),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { BOOST_ASSERT( impl_); }

public:
    pull_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( push_coroutine< R &, StackAllocator > &);

    explicit pull_coroutine( coroutine_fn fn,
                             attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_pull< coroutine_fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< coroutien_fn, impl_type > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    explicit pull_coroutine( coroutine_fn fn,
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
            detail::trampoline_pull< coroutine_fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< coroutien_fn, impl_type > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
# endif
    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
#else
    template< typename Fn >
    explicit pull_coroutine( Fn fn,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( Fn fn,
                             attributes const& attr,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, pull_coroutine >,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, pull_coroutine >,
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
            detail::trampoline_pull< Fn, impl_type, push_coroutine< R &, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
#endif

    ~pull_coroutine()
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    pull_coroutine( BOOST_RV_REF( pull_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    pull_coroutine & operator=( BOOST_RV_REF( pull_coroutine) other) BOOST_NOEXCEPT
    {
        pull_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( pull_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    pull_coroutine & operator()()
    {
        BOOST_ASSERT( * this);

        impl_->pull();
        return * this;
    }

    R & get() const
    { return impl_->get(); }

    class iterator : public std::iterator< std::input_iterator_tag, R >
    {
    private:
        pull_coroutine< R &, StackAllocator >   *   c_;
        R                                       *   val_;

        void fetch_()
        {
            BOOST_ASSERT( c_);

            if ( ! ( * c_) )
            {
                c_ = 0;
                val_ = 0;
                return;
            }
            val_ = c_->impl_->get_pointer();
        }

        void increment_()
        {
            BOOST_ASSERT( c_);
            BOOST_ASSERT( * c_);

            ( * c_)();
            fetch_();
        }

    public:
        typedef typename iterator::pointer      pointer_t;
        typedef typename iterator::reference    reference_t;

        iterator() :
            c_( 0), val_( 0)
        {}

        explicit iterator( pull_coroutine< R &, StackAllocator > * c) :
            c_( c), val_( 0)
        { fetch_(); }

        iterator( iterator const& other) :
            c_( other.c_), val_( other.val_)
        {}

        iterator & operator=( iterator const& other)
        {
            if ( this == & other) return * this;
            c_ = other.c_;
            val_ = other.val_;
            return * this;
        }

        bool operator==( iterator const& other)
        { return other.c_ == c_ && other.val_ == val_; }

        bool operator!=( iterator const& other)
        { return other.c_ != c_ || other.val_ != val_; }

        iterator & operator++()
        {
            increment_();
            return * this;
        }

        iterator operator++( int)
        {
            iterator tmp( * this);
            ++*this;
            return tmp;
        }

        reference_t operator*() const
        {
            if ( ! val_)
                boost::throw_exception(
                    invalid_result() );
            return * val_;
        }

        pointer_t operator->() const
        {
            if ( ! val_)
                boost::throw_exception(
                    invalid_result() );
            return val_;
        }
    };

    friend class iterator;

    struct const_iterator;
};

template< typename StackAllocator >
class pull_coroutine< void, StackAllocator >
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend void detail::trampoline_push_void( intptr_t);

    typedef detail::pull_coroutine_impl< void >     impl_type;
    typedef detail::parameters< void >              param_type;

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   caller_;
    detail::coroutine_context   callee_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( pull_coroutine)

    pull_coroutine( impl_type * impl) :
        impl_( impl),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { BOOST_ASSERT( impl_); }

public:
    pull_coroutine() BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( push_coroutine< void, StackAllocator > &);

    explicit pull_coroutine( coroutine_fn fn,
                             attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
                detail::trampoline_pull_void<
                    coroutine_fn, impl_type, push_coroutine< void, StackAllocator >
                >,
                & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn),
                                                  & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    explicit pull_coroutine( coroutine_fn fn,
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
            detail::trampoline_pull_void<
                coroutine_fn, impl_type, push_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn),
                                                  & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
# endif
    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes() ) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_pull_void<
                Fn, impl_type, push_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn),
                                        & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
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
            detail::trampoline_pull_void<
                Fn, impl_type, push_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( forward< Fn >( fn),
                                        & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
#else
    template< typename Fn >
    explicit pull_coroutine( Fn fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                        is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                        dummy* >::type = 0) :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_pull_void<
                Fn, impl_type, push_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( Fn fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                        is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                        dummy* >::type = 0) :
        impl_( 0),
        stack_alloc_( stack_alloc),
        stack_ctx_(),
        caller_(),
        callee_()
    {
        stack_alloc_.allocate( stack_ctx_, attr.size);
        callee_ = detail::coroutine_context(
            detail::trampoline_pull_void<
                Fn, impl_type, push_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr = attributes(),
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, pull_coroutine >,
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
            detail::trampoline_pull_void<
                Fn, impl_type, push_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }

    template< typename Fn >
    explicit pull_coroutine( BOOST_RV_REF( Fn) fn,
                             attributes const& attr,
                             StackAllocator const& stack_alloc,
                             typename disable_if<
                                 is_same< typename decay< Fn >::type, pull_coroutine >,
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
            detail::trampoline_pull_void<
                Fn, impl_type, push_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
        detail::setup< Fn > to( fn, & caller_, & callee_, attr);
        impl_ = reinterpret_cast< impl_type * >(
                caller_.jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    fpu_preserved == attr.preserve_fpu) );
        BOOST_ASSERT( impl_);
        impl_->pull();
    }
#endif

    ~pull_coroutine()
    {
        if ( 0 != stack_ctx_.sp)
        {
            BOOST_ASSERT( 0 != impl_);
            impl_->unwind_stack();
            stack_alloc_.deallocate( stack_ctx_);
            impl_ = 0;
        }
    }

    pull_coroutine( BOOST_RV_REF( pull_coroutine) other) BOOST_NOEXCEPT :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        caller_(),
        callee_()
    { swap( other); }

    pull_coroutine & operator=( BOOST_RV_REF( pull_coroutine) other) BOOST_NOEXCEPT
    {
        pull_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || impl_->is_complete(); }

    void swap( pull_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( caller_, other.caller_);
        std::swap( callee_, other.callee_);
    }

    pull_coroutine & operator()()
    {
        BOOST_ASSERT( * this);

        impl_->pull();
        return * this;
    }

    struct iterator;
    struct const_iterator;
};

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
template< typename Arg, typename StackAllocator >
push_coroutine< Arg, StackAllocator >::push_coroutine( coroutine_fn fn,
                                                       attributes const& attr) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
            detail::trampoline_push<
                coroutine_fn, impl_type, pull_coroutine< Arg, StackAllocator >
            >,
            & stack_ctx_);
    detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
push_coroutine< Arg, StackAllocator >::push_coroutine( coroutine_fn fn,
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
            detail::trampoline_push<
                coroutine_fn, impl_type, pull_coroutine< Arg, StackAllocator >
            >,
            & stack_ctx_);
    detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
push_coroutine< Arg &, StackAllocator >::push_coroutine( coroutine_fn fn,
                                                         attributes const& attr) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
            detail::trampoline_push<
                coroutine_fn, impl_type, pull_coroutine< Arg &, StackAllocator >
            >,
            & stack_ctx_);
    detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
push_coroutine< Arg &, StackAllocator >::push_coroutine( coroutine_fn fn,
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
            detail::trampoline_push<
                coroutine_fn, impl_type, pull_coroutine< Arg &, StackAllocator >
                coroutine_fn, impl_type, pull_coroutine< Arg &, StackAllocator >
            >,
            & stack_ctx_);
    detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
push_coroutine< void, StackAllocator >::push_coroutine( coroutine_fn fn,
                                                        attributes const& attr) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
            detail::trampoline_push_void<
                coroutine_fn, impl_type, pull_coroutine< void, StackAllocator >
            >,
            & stack_ctx_);
    detail::setup< coroutine_fn > to( forward< coroutine_fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
push_coroutine< void, StackAllocator >::push_coroutine( coroutine_fn fn,
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
        detail::trampoline_push_void<
            coroutine_fn, impl_type, pull_coroutine< void, StackAllocator >
        >,
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
template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                       attributes const& attr) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
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
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg &, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                         attributes const& attr) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg &, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg &, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
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
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg &, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( forward< Fn >( fn), & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
template< typename Fn >
push_coroutine< void, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                        attributes const& attr) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push_void<
            Fn, impl_type, pull_coroutine< void, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( forward< Fn >( fn),
                                    & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
template< typename Fn >
push_coroutine< void, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
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
        detail::trampoline_push_void<
            Fn, impl_type, pull_coroutine< void, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( forward< Fn >( fn),
                                    & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}
#else
template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg, StackAllocator >::push_coroutine( Fn fn,
                                                       attributes const& attr,
                                                       typename disable_if<
                                                            is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                                            dummy*
                                                       >::type) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg, StackAllocator >::push_coroutine( Fn fn,
                                                       attributes const& attr,
                                                       StackAllocator const& stack_alloc,
                                                       typename disable_if<
                                                            is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                                            dummy*
                                                       >::type) :
    impl_( 0),
    stack_alloc_( stack_alloc),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg &, StackAllocator >::push_coroutine( Fn fn,
                                                         attributes const& attr,
                                                         typename disable_if<
                                                              is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                                              dummy*
                                                         >::type) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg &, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg &, StackAllocator >::push_coroutine( Fn fn,
                                                         attributes const& attr,
                                                         StackAllocator const& stack_alloc,
                                                         typename disable_if<
                                                              is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                                              dummy*
                                                         >::type) :
    impl_( 0),
    stack_alloc_( stack_alloc),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg &, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
template< typename Fn >
push_coroutine< void, StackAllocator >::push_coroutine( Fn fn,
                                                        attributes const& attr,
                                                        typename disable_if<
                                                             is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                                             dummy*
                                                        >::type) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push_void<
            Fn, impl_type, pull_coroutine< void, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
template< typename Fn >
push_coroutine< void, StackAllocator >::push_coroutine( Fn fn,
                                                        attributes const& attr,
                                                        StackAllocator const& stack_alloc,
                                                        typename disable_if<
                                                             is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                                             dummy*
                                                        >::type) :
    impl_( 0),
    stack_alloc_( stack_alloc),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push_void<
            Fn, impl_type, pull_coroutine< void, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                       attributes const& attr,
                                                       typename disable_if<
                                                           is_same< typename decay< Fn >::type, push_coroutine >,
                                                           dummy*
                                                       >::type) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                       attributes const& attr,
                                                       StackAllocator const& stack_alloc,
                                                       typename disable_if<
                                                           is_same< typename decay< Fn >::type, push_coroutine >,
                                                           dummy*
                                                       >::type) :
    impl_( 0),
    stack_alloc_( stack_alloc),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg &, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                         attributes const& attr,
                                                         typename disable_if<
                                                             is_same< typename decay< Fn >::type, push_coroutine >,
                                                             dummy*
                                                         >::type) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg &, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename Arg, typename StackAllocator >
template< typename Fn >
push_coroutine< Arg &, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                         attributes const& attr,
                                                         StackAllocator const& stack_alloc,
                                                         typename disable_if<
                                                             is_same< typename decay< Fn >::type, push_coroutine >,
                                                             dummy*
                                                         >::type) :
    impl_( 0),
    stack_alloc_( stack_alloc),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push<
            Fn, impl_type, pull_coroutine< Arg &, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
template< typename Fn >
push_coroutine< void, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                        attributes const& attr,
                                                        typename disable_if<
                                                            is_same< typename decay< Fn >::type, push_coroutine >,
                                                            dummy*
                                                        >::type) :
    impl_( 0),
    stack_alloc_(),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push_void<
            Fn, impl_type, pull_coroutine< void, StackAllocator >
        >,
        & stack_ctx_);
    detail::setup< Fn > to( fn, & caller_, & callee_, attr);
    impl_ = reinterpret_cast< impl_type * >(
            caller_.jump(
                callee_,
                reinterpret_cast< intptr_t >( & to),
                fpu_preserved == attr.preserve_fpu) );
    BOOST_ASSERT( impl_);
}

template< typename StackAllocator >
template< typename Fn >
push_coroutine< void, StackAllocator >::push_coroutine( BOOST_RV_REF( Fn) fn,
                                                        attributes const& attr,
                                                        StackAllocator const& stack_alloc,
                                                        typename disable_if<
                                                            is_same< typename decay< Fn >::type, push_coroutine >,
                                                            dummy*
                                                        >::type) :
    impl_( 0),
    stack_alloc_( stack_alloc),
    stack_ctx_(),
    caller_(),
    callee_()
{
    stack_alloc_.allocate( stack_ctx_, attr.size);
    callee_ = detail::coroutine_context(
        detail::trampoline_push_void<
            Fn, impl_type, pull_coroutine< void, StackAllocator >
        >,
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

template< typename R, typename StackAllocator >
void swap( pull_coroutine< R, StackAllocator > & l, pull_coroutine< R, StackAllocator > & r) BOOST_NOEXCEPT
{ l.swap( r); }

template< typename Arg, typename StackAllocator >
void swap( push_coroutine< Arg, StackAllocator > & l, push_coroutine< Arg, StackAllocator > & r) BOOST_NOEXCEPT
{ l.swap( r); }

template< typename R, typename StackAllocator >
inline
typename pull_coroutine< R, StackAllocator >::iterator
range_begin( pull_coroutine< R, StackAllocator > & c)
{ return typename pull_coroutine< R, StackAllocator >::iterator( & c); }

template< typename R, typename StackAllocator >
inline
typename pull_coroutine< R, StackAllocator >::iterator
range_end( pull_coroutine< R, StackAllocator > &)
{ return typename pull_coroutine< R, StackAllocator >::iterator(); }

template< typename Arg, typename StackAllocator >
inline
typename push_coroutine< Arg, StackAllocator >::iterator
range_begin( push_coroutine< Arg, StackAllocator > & c)
{ return typename push_coroutine< Arg, StackAllocator >::iterator( & c); }

template< typename Arg, typename StackAllocator >
inline
typename push_coroutine< Arg, StackAllocator >::iterator
range_end( push_coroutine< Arg, StackAllocator > &)
{ return typename push_coroutine< Arg, StackAllocator >::iterator(); }

template< typename T, typename StackAllocator = stack_allocator >
struct asymmetric_coroutine
{
    typedef push_coroutine< T, StackAllocator > push_type;
    typedef pull_coroutine< T, StackAllocator > pull_type;
};

// deprecated
template< typename T, typename StackAllocator = stack_allocator >
struct coroutine
{
    typedef push_coroutine< T, StackAllocator > push_type;
    typedef pull_coroutine< T, StackAllocator > pull_type;
};

}

template< typename Arg, typename StackAllocator >
struct range_mutable_iterator< coroutines::push_coroutine< Arg, StackAllocator > >
{ typedef typename coroutines::push_coroutine< Arg, StackAllocator >::iterator type; };

template< typename R, typename StackAllocator >
struct range_mutable_iterator< coroutines::pull_coroutine< R, StackAllocator > >
{ typedef typename coroutines::pull_coroutine< R, StackAllocator >::iterator type; };

}

namespace std {

template< typename R, typename StackAllocator >
inline
typename boost::coroutines::pull_coroutine< R, StackAllocator >::iterator
begin( boost::coroutines::pull_coroutine< R, StackAllocator > & c)
{ return boost::begin( c); }

template< typename R, typename StackAllocator >
inline
typename boost::coroutines::pull_coroutine< R, StackAllocator >::iterator
end( boost::coroutines::pull_coroutine< R, StackAllocator > & c)
{ return boost::end( c); }

template< typename R, typename StackAllocator >
inline
typename boost::coroutines::push_coroutine< R, StackAllocator >::iterator
begin( boost::coroutines::push_coroutine< R, StackAllocator > & c)
{ return boost::begin( c); }

template< typename R, typename StackAllocator >
inline
typename boost::coroutines::push_coroutine< R, StackAllocator >::iterator
end( boost::coroutines::push_coroutine< R, StackAllocator > & c)
{ return boost::end( c); }

}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_ASYMMETRIC_COROUTINE_H
