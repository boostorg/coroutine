
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBER_PREALLOCATED_STACK_ALLOCATOR_H
#define BOOST_FIBER_PREALLOCATED_STACK_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/coroutine/stack_context.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <boost/fiber/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

template< std::size_t Default >
class preallocated_stack_allocator
{
private:
    class impl : private boost::noncopyable
    {
    private:
        std::size_t             pos_;
        std::vector< void * >   stacks_;

    public:
        impl( std::size_t count) :
            pos_( 0),
            stacks_()
        {
            for ( int i = 0; i < count; ++i)
            {
                void * limit = std::calloc( preallocated_stack_allocator::default_stacksize(), sizeof( char) );
                if ( ! limit) throw std::bad_alloc();
                limit = static_cast< char * >( limit) + preallocated_stack_allocator::default_stacksize();
                stacks_.push_back( limit);
            }
        }

        ~impl()
        {
            for ( int i = 0; i < stacks_.size(); ++i)
            {
                void * limit = static_cast< char * >( stacks_[i]);
                limit = static_cast< char * >( limit) - preallocated_stack_allocator::default_stacksize();
                std::free( limit);
            }
        }

        void allocate( boost::coroutines::stack_context & ctx, std::size_t size)
        {
            BOOST_ASSERT( stacks_.size() > pos_);
            BOOST_ASSERT( preallocated_stack_allocator::minimum_stacksize() <= size);
            BOOST_ASSERT( preallocated_stack_allocator::maximum_stacksize() >= size);

            ctx.size = preallocated_stack_allocator::default_stacksize();
            ctx.sp = stacks_[pos_++];
        }

        void deallocate( boost::coroutines::stack_context & ctx)
        {
            BOOST_ASSERT( ctx.sp);
            BOOST_ASSERT( preallocated_stack_allocator::minimum_stacksize() <= ctx.size);
            BOOST_ASSERT( preallocated_stack_allocator::maximum_stacksize() >= ctx.size);
        }
    };

    boost::shared_ptr< impl >   p_;

public:
    static std::size_t maximum_stacksize()
    { return Default; }

    static std::size_t default_stacksize()
    { return Default; }

    static std::size_t minimum_stacksize()
    { return Default; }

    preallocated_stack_allocator( std::size_t count) :
        p_( new impl( count) )
    {}

    void allocate( boost::coroutines::stack_context & ctx, std::size_t size)
    { p_->allocate( ctx, size); }

    void deallocate( boost::coroutines::stack_context & ctx)
    { p_->deallocate( ctx); }
};

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBER_PREALLOCATED_STACK_ALLOCATOR_H
