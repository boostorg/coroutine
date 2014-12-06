
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_NULL_STACK_ALLOCATOR_H
#define BOOST_COROUTINES_NULL_STACK_ALLOCATOR_H

extern "C" {
#include <windows.h>
}

#include <cmath>
#include <cstddef>
#include <new>

#include <boost/config.hpp>

#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/stack_traits.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

struct stack_context;

template< typename traitsT >
struct basic_null_stack_allocator
{
    typedef traitsT traits_type;

    void allocate(stack_context & ctx, std::size_t size_)
    {
        // just to avoid asserts, the stack is not used by Fiber anyway
        ctx.size = size_;
        ctx.sp = static_cast< char * >(1) + ctx.size;
    }

    void deallocate(stack_context & ctx)
    {}
};

typedef basic_null_stack_allocator< stack_traits > null_stack_allocator;

}
}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_NULL_STACK_ALLOCATOR_H
