
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_PARAMETERS_H
#define BOOST_COROUTINES_DETAIL_PARAMETERS_H

#include <boost/assert.hpp>
#include <boost/config.hpp>

#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/coroutine/detail/flags.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {
namespace detail {

template< typename Data >
struct parameters
{
    coroutine_context   *   ctx;
    Data                *   data;
    bool                    do_unwind;

    explicit parameters( coroutine_context * ctx_) :
        ctx( ctx_), data( 0), do_unwind( false)
    { BOOST_ASSERT( ctx); }

    explicit parameters( coroutine_context * ctx_, Data * data_) :
        ctx( ctx_), data( data_), do_unwind( false)
    { BOOST_ASSERT( ctx); }

    explicit parameters( coroutine_context * ctx_, unwind_t::flag_t) :
        ctx( ctx_), data( 0), do_unwind( true)
    {
        BOOST_ASSERT( ctx);
        BOOST_ASSERT( do_unwind);
    }

    parameters( parameters const& other) :
        ctx( other.ctx), data( other.data),
        do_unwind( other.do_unwind)
    {}

    parameters & operator=( parameters const& other)
    {
        if ( this == & other) return * this;
        ctx = other.ctx;
        data = other.data;
        do_unwind = other.do_unwind;
        return * this;
    }
};

template< typename Data >
struct parameters< Data & >
{
    coroutine_context   *   ctx;
    Data                *   data;
    bool                    do_unwind;

    explicit parameters( coroutine_context * ctx_) :
        ctx( ctx_), data( 0), do_unwind( false)
    { BOOST_ASSERT( ctx); }

    explicit parameters( coroutine_context * ctx_, Data * data_) :
        ctx( ctx_), data( data_), do_unwind( false)
    { BOOST_ASSERT( ctx); }

    explicit parameters( coroutine_context * ctx_, unwind_t::flag_t) :
        ctx( ctx_), data( 0), do_unwind( true)
    {
        BOOST_ASSERT( ctx);
        BOOST_ASSERT( do_unwind);
    }

    parameters( parameters const& other) :
        ctx( other.ctx), data( other.data),
        do_unwind( other.do_unwind)
    {}

    parameters & operator=( parameters const& other)
    {
        if ( this == & other) return * this;
        ctx = other.ctx;
        data = other.data;
        do_unwind = other.do_unwind;
        return * this;
    }
};

template< typename Data >
struct parameters< Data * >
{
    coroutine_context   *   ctx;
    Data                **  data;
    bool                    do_unwind;

    explicit parameters( coroutine_context * ctx_) :
        ctx( ctx_), data( 0), do_unwind( false)
    { BOOST_ASSERT( ctx); }

    explicit parameters( coroutine_context * ctx_, Data ** data_) :
        ctx( ctx_), data( data_), do_unwind( false)
    { BOOST_ASSERT( ctx); }

    explicit parameters( coroutine_context * ctx_, unwind_t::flag_t) :
        ctx( ctx_), data( 0), do_unwind( true)
    {
        BOOST_ASSERT( ctx);
        BOOST_ASSERT( do_unwind);
    }

    parameters( parameters const& other) :
        ctx( other.ctx), data( other.data),
        do_unwind( other.do_unwind)
    {}

    parameters & operator=( parameters const& other)
    {
        if ( this == & other) return * this;
        ctx = other.ctx;
        data = other.data;
        do_unwind = other.do_unwind;
        return * this;
    }
};

template<>
struct parameters< void >
{
    coroutine_context   *   ctx;
    bool                    do_unwind;

    explicit parameters( coroutine_context * ctx_) :
        ctx( ctx_), do_unwind( false)
    { BOOST_ASSERT( ctx); }

    explicit parameters( coroutine_context * ctx_, unwind_t::flag_t) :
        ctx( ctx_), do_unwind( true)
    { BOOST_ASSERT( ctx); }

    parameters( parameters const& other) :
        ctx( other.ctx), do_unwind( other.do_unwind)
    {}

    parameters & operator=( parameters const& other)
    {
        if ( this == & other) return * this;
        ctx = other.ctx;
        do_unwind = other.do_unwind;
        return * this;
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_PARAMETERS_H
