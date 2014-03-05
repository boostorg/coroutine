
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/coroutine/standard_stack_allocator.hpp>

#include <cstdlib>
#include <stdexcept>

#include <boost/assert.hpp>

#include <boost/coroutine/stack_context.hpp>
#include <boost/coroutine/protected_stack_allocator.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

bool
standard_stack_allocator::is_stack_unbounded()
{ return protected_stack_allocator::is_stack_unbounded(); }

std::size_t
standard_stack_allocator::maximum_stacksize()
{ return protected_stack_allocator::maximum_stacksize(); }

std::size_t
standard_stack_allocator::default_stacksize()
{ return protected_stack_allocator::default_stacksize(); }

std::size_t
standard_stack_allocator::minimum_stacksize()
{ return protected_stack_allocator::minimum_stacksize(); }

void
standard_stack_allocator::allocate( stack_context & ctx, std::size_t size)
{
    BOOST_ASSERT( minimum_stacksize() <= size);
    BOOST_ASSERT( is_stack_unbounded() || ( maximum_stacksize() >= size) );

    void * limit = std::malloc( size);
    if ( ! limit) throw std::bad_alloc();

    ctx.size = size;
    ctx.sp = static_cast< char * >( limit) + ctx.size;
}

void
standard_stack_allocator::deallocate( stack_context & ctx)
{
    BOOST_ASSERT( ctx.sp);
    BOOST_ASSERT( minimum_stacksize() <= ctx.size);
    BOOST_ASSERT( is_stack_unbounded() || ( maximum_stacksize() >= ctx.size) );

    void * limit = static_cast< char * >( ctx.sp) - ctx.size;
    std::free( limit);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
