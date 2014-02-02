
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_PARAMETERS_H
#define BOOST_COROUTINES_DETAIL_PARAMETERS_H

#include <boost/assert.hpp>
#include <boost/config.hpp>

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
    Data                *   data;
    bool                    do_unwind;

    parameters() :
        data( 0), do_unwind( false)
    {}

    explicit parameters( Data * data_) :
        data( data_), do_unwind( false)
    { BOOST_ASSERT( data); }

    explicit parameters( unwind_t::flag_t) :
        data( 0), do_unwind( true)
    {}
};

template< typename Data >
struct parameters< Data & >
{
    Data                *   data;
    bool                    do_unwind;

    parameters() :
        data( 0), do_unwind( false)
    {}

    explicit parameters( Data * data_) :
        data( data_), do_unwind( false)
    { BOOST_ASSERT( data); }

    explicit parameters( unwind_t::flag_t) :
        data( 0), do_unwind( true)
    {}
};

template<>
struct parameters< void >
{
    bool                    do_unwind;

    parameters() :
        do_unwind( false)
    {}

    explicit parameters( unwind_t::flag_t) :
        do_unwind( true)
    {}
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_PARAMETERS_H
