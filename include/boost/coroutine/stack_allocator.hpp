
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_STACK_ALLOCATOR_H
#define BOOST_COROUTINES_STACK_ALLOCATOR_H

#include <cstddef>

#include <boost/config.hpp>

#include <boost/context/detail/config.hpp>
#include <boost/coroutine/segmented_stack_allocator.hpp>
#include <boost/coroutine/standard_stack_allocator.hpp>
#if defined(BOOST_WINDOWS)
# include <boost/coroutine/windows/null_stack_allocator.hpp>
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

#if defined(BOOST_USE_SEGMENTED_STACKS)
typedef segmented_stack_allocator   stack_allocator;
#elif defined(BOOST_COROUTINE_USE_FIBER)
typedef null_stack_allocator    stack_allocator;
#else
typedef standard_stack_allocator    stack_allocator;
#endif

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_STACK_ALLOCATOR_H
