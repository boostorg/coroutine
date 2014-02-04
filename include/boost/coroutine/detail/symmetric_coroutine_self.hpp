
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_SELF_H
#define BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_SELF_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/utility/explicit_operator_bool.hpp>
#include <boost/throw_exception.hpp>

#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/coroutine/detail/flags.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/exceptions.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

struct stack_context;

namespace detail {

template< typename R >
class symmetric_coroutine_self
{
private:
    typedef parameters< R >                     param_type;
    typedef symmetric_coroutine_impl< R >       impl_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine_self)

    impl_type   *   impl_;
    R           *   result_;

public:
    symmetric_coroutine_self() BOOST_NOEXCEPT :
        impl_( 0),
        result_( 0)
    {}

    symmetric_coroutine_self( impl_type * impl) BOOST_NOEXCEPT :
        impl_( impl),
        result_( 0)
    { BOOST_ASSERT( impl_); }

    symmetric_coroutine_self( impl_type * impl, R * result) BOOST_NOEXCEPT :
        impl_( impl),
        result_( result)
    { BOOST_ASSERT( impl_); }

    symmetric_coroutine_self( BOOST_RV_REF( symmetric_coroutine_self) other) BOOST_NOEXCEPT :
        impl_( 0),
        result_( 0)
    { swap( other); }

    symmetric_coroutine_self & operator=( BOOST_RV_REF( symmetric_coroutine_self) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine_self tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || 0 == result_; }

    void swap( symmetric_coroutine_self & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( result_, other.result_);
    }

    symmetric_coroutine_self & operator()()
    {
        result_ = impl_->yield();
        return * this;
    }

    template< typename Coro >
    symmetric_coroutine_self & operator()( Coro & other, typename Coro::value_type & x)
    {
        BOOST_ASSERT( other);

        result_ = impl_->yield_to( other.impl_, x);
        return * this;
    }

    template< typename Coro >
    symmetric_coroutine_self & operator()( Coro & other)
    {
        BOOST_ASSERT( other);

        result_ = impl_->yield_to( other.impl_);
        return * this;
    }

    R get() const
    {
        if ( ! * this)
            boost::throw_exception(
                invalid_result() );
        return * result_; 
    }
};

template< typename R >
class symmetric_coroutine_self< R & >
{
private:
    typedef parameters< R & >                   param_type;
    typedef symmetric_coroutine_impl< R & >     impl_type;

    struct dummy {};

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine_self)

    impl_type   *   impl_;
    R           *   result_;

public:
    symmetric_coroutine_self() BOOST_NOEXCEPT :
        impl_( 0),
        result_( 0)
    {}

    symmetric_coroutine_self( impl_type * impl) BOOST_NOEXCEPT :
        impl_( impl),
        result_( 0)
    { BOOST_ASSERT( impl_); }

    symmetric_coroutine_self( impl_type * impl, R * result) BOOST_NOEXCEPT :
        impl_( impl),
        result_( result)
    { BOOST_ASSERT( impl_); }

    symmetric_coroutine_self( BOOST_RV_REF( symmetric_coroutine_self) other) BOOST_NOEXCEPT :
        impl_( 0),
        result_( 0)
    { swap( other); }

    symmetric_coroutine_self & operator=( BOOST_RV_REF( symmetric_coroutine_self) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine_self tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 == impl_ || 0 == result_; }

    void swap( symmetric_coroutine_self & other) BOOST_NOEXCEPT
    {
        std::swap( impl_, other.impl_);
        std::swap( result_, other.result_);
    }

    symmetric_coroutine_self & operator()()
    {
        result_ = impl_->yield();
        return * this;
    }

    template< typename Coro >
    symmetric_coroutine_self & operator()( Coro & other, typename Coro::value_type & x)
    {
        BOOST_ASSERT( other);

        result_ = impl_->yield_to( other.impl_, x);
        return * this;
    }

    template< typename Coro >
    symmetric_coroutine_self & operator()( Coro & other)
    {
        BOOST_ASSERT( other);

        result_ = impl_->yield_to( other.impl_);
        return * this;
    }

    R & get() const
    {
        if ( ! * this)
            boost::throw_exception(
                invalid_result() );
        return * result_; 
    }
};

template<>
class symmetric_coroutine_self< void >
{
private:
    typedef parameters< void >                  param_type;
    typedef symmetric_coroutine_impl< void >    impl_type;

    BOOST_MOVABLE_BUT_NOT_COPYABLE( symmetric_coroutine_self)

    impl_type   *   impl_;

public:
    symmetric_coroutine_self() BOOST_NOEXCEPT :
        impl_( 0)
    {}

    symmetric_coroutine_self( impl_type * impl) BOOST_NOEXCEPT :
        impl_( impl)
    { BOOST_ASSERT( impl_); }

    symmetric_coroutine_self( BOOST_RV_REF( symmetric_coroutine_self) other) BOOST_NOEXCEPT :
        impl_( 0)
    { swap( other); }

    symmetric_coroutine_self & operator=( BOOST_RV_REF( symmetric_coroutine_self) other) BOOST_NOEXCEPT
    {
        symmetric_coroutine_self tmp( boost::move( other) );
        swap( tmp);
        return * this;
    }

    BOOST_EXPLICIT_OPERATOR_BOOL();

    bool operator!() const BOOST_NOEXCEPT
    { return 0 != impl_; }

    void swap( symmetric_coroutine_self & other) BOOST_NOEXCEPT
    { std::swap( impl_, other.impl_); }

    symmetric_coroutine_self & operator()()
    {
        impl_->yield();
        return * this;
    }

    template< typename Coro >
    symmetric_coroutine_self & operator()( Coro & other, typename Coro::value_type & x)
    {
        BOOST_ASSERT( other);

        impl_->yield_to( other.impl_, x);
        return * this;
    }

    template< typename Coro >
    symmetric_coroutine_self & operator()( Coro & other)
    {
        BOOST_ASSERT( other);

        impl_->yield_to( other.impl_);
        return * this;
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_SYMMETRIC_COROUTINE_SELF_H
