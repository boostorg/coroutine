
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/coroutine/protected_stack_allocator.hpp"

extern "C" {
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
}

//#if _POSIX_C_SOURCE >= 200112L

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include <boost/assert.hpp>
#include <boost/context/fcontext.hpp>
#include <boost/thread.hpp>

#include <boost/coroutine/stack_context.hpp>

#if !defined (SIGSTKSZ)
# define SIGSTKSZ (8 * 1024)
# define UDEF_SIGSTKSZ
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {

void pagesize_( std::size_t * size)
{
    // conform to POSIX.1-2001
    * size = ::sysconf( _SC_PAGESIZE);
}

void stacksize_limit_( rlimit * limit)
{
    // conforming to POSIX.1-2001
#if defined(BOOST_DISABLE_ASSERTS)
    ::getrlimit( RLIMIT_STACK, limit);
#else
    const int result = ::getrlimit( RLIMIT_STACK, limit);
    BOOST_ASSERT( 0 == result);
#endif
}

// conform to POSIX.1-2001
std::size_t pagesize()
{
    static std::size_t size = 0;
    static boost::once_flag flag;
    boost::call_once( flag, pagesize_, & size);
    return size;
}

rlimit stacksize_limit()
{
    static rlimit limit;
    static boost::once_flag flag;
    boost::call_once( flag, stacksize_limit_, & limit);
    return limit;
}

std::size_t page_count( std::size_t stacksize)
{
    return static_cast< std::size_t >( 
        std::floor(
            static_cast< float >( stacksize) / pagesize() ) );
}

bool
protected_stack_allocator::is_stack_unbound()
{ return RLIM_INFINITY == stacksize_limit().rlim_max; }

std::size_t
protected_stack_allocator::default_stacksize()
{
    std::size_t size = 8 * minimum_stacksize();
    if ( is_stack_unbound() ) return size;

    BOOST_ASSERT( maximum_stacksize() >= minimum_stacksize() );
    return maximum_stacksize() == size
        ? size
        : (std::min)( size, maximum_stacksize() );
}

std::size_t
protected_stack_allocator::minimum_stacksize()
{ return SIGSTKSZ + sizeof( context::fcontext_t) + 15; }

std::size_t
protected_stack_allocator::maximum_stacksize()
{
    BOOST_ASSERT( ! is_stack_unbound() );
    return static_cast< std::size_t >( stacksize_limit().rlim_max);
}

void
protected_stack_allocator::allocate( stack_context & ctx, std::size_t size)
{
    BOOST_ASSERT( minimum_stacksize() <= size);
    BOOST_ASSERT( is_stack_unbound() || ( maximum_stacksize() >= size) );

    const std::size_t pages( page_count( size) ); // page at bottom will be used as guard-page
    BOOST_ASSERT_MSG( 2 <= pages, "at least two pages must fit into stack (one page is guard-page)");
    const std::size_t size_( pages * pagesize() );
    BOOST_ASSERT( 0 < size && 0 < size_);
    BOOST_ASSERT( size_ <= size);

    const int fd( ::open("/dev/zero", O_RDONLY) );
    BOOST_ASSERT( -1 != fd);
    // conform to POSIX.4 (POSIX.1b-1993, _POSIX_C_SOURCE=199309L)
    void * limit =
# if defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    ::mmap( 0, size_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
# else
    ::mmap( 0, size_, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
# endif
    ::close( fd);
    if ( ! limit) throw std::bad_alloc();

    std::memset( limit, '\0', size_);

    // conforming to POSIX.1-2001
#if defined(BOOST_DISABLE_ASSERTS)
    ::mprotect( limit, pagesize(), PROT_NONE);
#else
    const int result( ::mprotect( limit, pagesize(), PROT_NONE) );
    BOOST_ASSERT( 0 == result);
#endif

    ctx.size = size_;
    ctx.sp = static_cast< char * >( limit) + ctx.size;
}

void
protected_stack_allocator::deallocate( stack_context & ctx)
{
    BOOST_ASSERT( ctx.sp);
    BOOST_ASSERT( minimum_stacksize() <= ctx.size);
    BOOST_ASSERT( is_stack_unbound() || ( maximum_stacksize() >= ctx.size) );

    void * limit = static_cast< char * >( ctx.sp) - ctx.size;
    // conform to POSIX.4 (POSIX.1b-1993, _POSIX_C_SOURCE=199309L)
    ::munmap( limit, ctx.size);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#ifdef UDEF_SIGSTKSZ
# undef SIGSTKSZ
#endif
