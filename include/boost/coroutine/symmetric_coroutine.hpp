
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_SYMMETRIC_COROUTINE_H
#define BOOST_COROUTINES_SYMMETRIC_COROUTINE_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/utility/explicit_operator_bool.hpp>

#include <boost/coroutine/attributes.hpp>
#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/detail/setup.hpp>
#include <boost/coroutine/detail/symmetric_coroutine_impl.hpp>
#include <boost/coroutine/detail/symmetric_coroutine_self.hpp>
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
    typedef detail::symmetric_coroutine_impl< T >   impl_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine)

    struct dummy {};

    impl_type               *   impl_;
    StackAllocator              stack_alloc_;
    stack_context               stack_ctx_;
    detail::coroutine_context   coro_ctx_;

public:
    typedef detail::symmetric_coroutine_self< T >   self_type;

    symmetric_coroutine() :
        impl_( 0),
        stack_alloc_(),
        stack_ctx_(),
        coro_ctx_()
    {}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
# ifdef BOOST_MSVC
	typedef void ( * coroutine_fn)( symmetric_coroutine< T, StackAllocator > &);

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr = attributes() );

    explicit symmetric_coroutine( coroutine_fn fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc);
# endif
    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes() );

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc);
#else
    template< typename Fn >
    explicit symmetric_coroutine( Fn fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                     dummy*
                                  >::type = 0);

    template< typename Fn >
    explicit symmetric_coroutine( Fn fn, attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_convertible< Fn&, BOOST_RV_REF(Fn) >,
                                      dummy*
                                  >::type = 0);

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr = attributes(),
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0);

    template< typename Fn >
    explicit symmetric_coroutine( BOOST_RV_REF( Fn) fn,
                                  attributes const& attr,
                                  StackAllocator const& stack_alloc,
                                  typename disable_if<
                                      is_same< typename decay< Fn >::type, symmetric_coroutine >,
                                      dummy*
                                  >::type = 0);
#endif

    ~symmetric_coroutine()
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
        coro_ctx_()
    { swap( other); }

    symmetric_coroutine & operator=( BOOST_RV_REF( symmetric_coroutine) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    bool empty() const BOOST_NOEXCEPT
    { return 0 == impl_; }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return empty() || impl_->is_complete(); }

    void swap( symmetric_coroutine & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( stack_alloc_, other.stack_alloc_);
        std::swap( stack_ctx_, other.stack_ctx_);
        std::swap( coro_ctx_, other.coro_ctx_);
    }
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_SYMMETRIC_COROUTINE_H
