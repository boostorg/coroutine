
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/coroutine/detail/coroutine_context.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4355)
#endif

#if defined(BOOST_USE_SEGMENTED_STACKS)
extern "C" {

void __splitstack_getcontext( void * [BOOST_COROUTINES_SEGMENTS]);

void __splitstack_setcontext( void * [BOOST_COROUTINES_SEGMENTS]);

}
#endif

namespace boost {
namespace coroutines {
namespace detail {
#ifdef BOOST_COROUTINE_USE_FIBER
VOID WINAPI coroutine_context::fb_start_proc(LPVOID lpFiberParameter)
{
    coroutine_context * ctx = (coroutine_context*)lpFiberParameter;
    ctx->fn_(ctx->param_);
    DeleteFiber(ctx->fiber_);
}
#endif

coroutine_context::coroutine_context() :
    stack_ctx_(),
    ctx_( 0)
{
#if defined(BOOST_USE_SEGMENTED_STACKS)
    __splitstack_getcontext( stack_ctx_.segments_ctx);
#elif defined(BOOST_COROUTINE_USE_FIBER)
    fiber_ = NULL;
    param_ = 0;
    fn_ = 0;
#endif
}

coroutine_context::coroutine_context( ctx_fn fn, stack_context const& stack_ctx) :
    stack_ctx_( stack_ctx)
#ifndef BOOST_COROUTINE_USE_FIBER
    , ctx_(context::make_fcontext( stack_ctx_.sp, stack_ctx_.size, fn) )
#endif // !BOOST_COROUTINE_USE_FIBER
{
#ifdef BOOST_COROUTINE_USE_FIBER
    fn_ = fn;
    fiber_ = CreateFiber(0, fb_start_proc, this);
#endif // BOOST_COROUTINE_USE_FIBER
}

coroutine_context::coroutine_context( coroutine_context const& other) :
    stack_ctx_( other.stack_ctx_),
    ctx_( other.ctx_)
{
#ifdef BOOST_COROUTINE_USE_FIBER
    fn_ = other.fn_;
    fiber_ = other.fiber_;
    param_ = other.param_;
#endif // BOOST_COROUTINE_USE_FIBER
}

coroutine_context &
coroutine_context::operator=( coroutine_context const& other)
{
    if ( this == & other) return * this;

    stack_ctx_ = other.stack_ctx_;
    ctx_ = other.ctx_;
#ifdef BOOST_COROUTINE_USE_FIBER
    fn_ = other.fn_;
    fiber_ = other.fiber_;
    param_ = other.param_;
#endif // BOOST_COROUTINE_USE_FIBER
    return * this;
}

intptr_t
coroutine_context::jump( coroutine_context & other, intptr_t param, bool preserve_fpu)
{
#if defined(BOOST_USE_SEGMENTED_STACKS)
    __splitstack_getcontext( stack_ctx_.segments_ctx);
    __splitstack_setcontext( other.stack_ctx_.segments_ctx);

    intptr_t ret = context::jump_fcontext( & ctx_, other.ctx_, param, preserve_fpu);

    __splitstack_setcontext( stack_ctx_.segments_ctx);

    return ret;
#elif defined(BOOST_COROUTINE_USE_FIBER)
    other.param_ = param;
#ifdef BOOST_COROUTINE_USE_IS_THREAD_A_FIBER
    if (!IsThreadAFiber())
    {
        this->fiber_ = ConvertThreadToFiber(this);
        SwitchToFiber(other.fiber_);
        ConvertFiberToThread();
        this->fiber_ = NULL;
    }
    else
#endif // BOOST_COROUTINE_USE_IS_THREAD_A_FIBER
    {
        this->fiber_ = GetCurrentFiber();
        SwitchToFiber(other.fiber_);
    }
    return this->param_;
#else
    return context::jump_fcontext( & ctx_, other.ctx_, param, preserve_fpu);
#endif
}

void coroutine_context::destory()
{
#ifdef BOOST_COROUTINE_USE_FIBER
    DeleteFiber(fiber_);
#endif
}

}}}

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
