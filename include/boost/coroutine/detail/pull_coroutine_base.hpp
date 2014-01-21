
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_PULL_COROUTINE_BASE_H
#define BOOST_COROUTINES_DETAIL_PULL_COROUTINE_BASE_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/context/fcontext.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility.hpp>

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
class pull_coroutine_base : private noncopyable
{
public:
    typedef intrusive_ptr< pull_coroutine_base >     ptr_t;

private:
    template<
        typename X, typename Y, typename Z, typename V, typename W
    >
    friend class push_coroutine_object;

    typedef parameters< R >                           param_type;

    unsigned int        use_count_;

protected:
    int                 flags_;
    exception_ptr       except_;
    coroutine_context   caller_;
    coroutine_context   callee_;
    R               *   result_;

    virtual void deallocate_object() = 0;

public:
    pull_coroutine_base( coroutine_context::ctx_fn fn,
                         stack_context * stack_ctx,
                         bool unwind, bool preserve_fpu) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( fn, stack_ctx),
        result_( 0)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    pull_coroutine_base( coroutine_context const& callee,
                         bool unwind, bool preserve_fpu,
                         R * result) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( callee),
        result_( result)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    virtual ~pull_coroutine_base()
    {}

    bool force_unwind() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_force_unwind); }

    bool unwind_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_unwind_stack); }

    bool preserve_fpu() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_preserve_fpu); }

    bool is_complete() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_complete); }

    void pull()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( & caller_);
        param_type * from(
            reinterpret_cast< param_type * >(
                to.ctx->jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        BOOST_ASSERT( from->ctx);
        callee_ = * from->ctx;
        result_ = from->data;
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }

    bool has_result() const
    { return 0 != result_; }

    R get() const
    {
        if ( ! has_result() )
            boost::throw_exception(
                invalid_result() );
        return * result_; 
    }

    R * get_pointer() const
    {
        if ( ! has_result() )
            boost::throw_exception(
                invalid_result() );
        return result_; 
    }

    friend inline void intrusive_ptr_add_ref( pull_coroutine_base * p) BOOST_NOEXCEPT
    { ++p->use_count_; }

    friend inline void intrusive_ptr_release( pull_coroutine_base * p) BOOST_NOEXCEPT
    { if ( --p->use_count_ == 0) p->deallocate_object(); }
};

template< typename R >
class pull_coroutine_base< R * > : private noncopyable
{
public:
    typedef intrusive_ptr< pull_coroutine_base >    ptr_t;

private:
    template<
        typename X, typename Y, typename Z, typename V, typename W
    >
    friend class push_coroutine_object;

    typedef parameters< R * >                       param_type;

    unsigned int        use_count_;

protected:
    int                 flags_;
    exception_ptr       except_;
    coroutine_context   caller_;
    coroutine_context   callee_;
    R               **  result_;

    virtual void deallocate_object() = 0;

public:
    pull_coroutine_base( coroutine_context::ctx_fn fn,
                         stack_context * stack_ctx,
                         bool unwind, bool preserve_fpu) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( fn, stack_ctx),
        result_( 0)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    pull_coroutine_base( coroutine_context const& callee,
                         bool unwind, bool preserve_fpu,
                         R ** result) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( callee),
        result_( result)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    virtual ~pull_coroutine_base()
    {}

    bool force_unwind() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_force_unwind); }

    bool unwind_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_unwind_stack); }

    bool preserve_fpu() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_preserve_fpu); }

    bool is_complete() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_complete); }

    void pull()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( & caller_);
        param_type * from(
            reinterpret_cast< param_type * >(
                to.ctx->jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        BOOST_ASSERT( from->ctx);
        callee_ = * from->ctx;
        result_ = from->data;
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }

    bool has_result() const
    { return 0 != result_; }

    R * get() const
    {
        if ( ! has_result() )
            boost::throw_exception(
                invalid_result() );
        return * result_;
    }

    R ** get_pointer() const
    {
        if ( ! has_result() )
            boost::throw_exception(
                invalid_result() );
        return result_;
    }

    friend inline void intrusive_ptr_add_ref( pull_coroutine_base * p) BOOST_NOEXCEPT
    { ++p->use_count_; }

    friend inline void intrusive_ptr_release( pull_coroutine_base * p) BOOST_NOEXCEPT
    { if ( --p->use_count_ == 0) p->deallocate_object(); }
};

template< typename R >
class pull_coroutine_base< R & > : private noncopyable
{
public:
    typedef intrusive_ptr< pull_coroutine_base >        ptr_t;

private:
    template<
        typename X, typename Y, typename Z, typename V, typename W
    >
    friend class push_coroutine_object;

    typedef parameters< R & >                           param_type;

    unsigned int        use_count_;

protected:
    int                 flags_;
    exception_ptr       except_;
    coroutine_context   caller_;
    coroutine_context   callee_;
    R               *   result_;

    virtual void deallocate_object() = 0;

public:
    pull_coroutine_base( coroutine_context::ctx_fn fn,
                         stack_context * stack_ctx,
                         bool unwind, bool preserve_fpu) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( fn, stack_ctx),
        result_( 0)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    pull_coroutine_base( coroutine_context const& callee,
                         bool unwind, bool preserve_fpu,
                         R * result) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( callee),
        result_( result)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    virtual ~pull_coroutine_base()
    {}

    bool force_unwind() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_force_unwind); }

    bool unwind_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_unwind_stack); }

    bool preserve_fpu() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_preserve_fpu); }

    bool is_complete() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_complete); }

    void pull()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( & caller_);
        param_type * from(
            reinterpret_cast< param_type * >(
                to.ctx->jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        BOOST_ASSERT( from->ctx);
        callee_ = * from->ctx;
        result_ = from->data;
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }

    bool has_result() const
    { return 0 != result_; }

    R & get() const
    {
        if ( ! has_result() )
            boost::throw_exception(
                invalid_result() );
        return * result_;
    }

    R * get_pointer() const
    {
        if ( ! has_result() )
            boost::throw_exception(
                invalid_result() );
        return result_;
    }

    friend inline void intrusive_ptr_add_ref( pull_coroutine_base * p) BOOST_NOEXCEPT
    { ++p->use_count_; }

    friend inline void intrusive_ptr_release( pull_coroutine_base * p) BOOST_NOEXCEPT
    { if ( --p->use_count_ == 0) p->deallocate_object(); }
};

template<>
class pull_coroutine_base< void > : private noncopyable
{
public:
    typedef intrusive_ptr< pull_coroutine_base >        ptr_t;

private:
    template<
        typename X, typename Y, typename Z, typename V, typename W
    >
    friend class push_coroutine_object;

    typedef parameters< void >                           param_type;

    unsigned int        use_count_;

protected:
    int                 flags_;
    exception_ptr       except_;
    coroutine_context   caller_;
    coroutine_context   callee_;

    virtual void deallocate_object() = 0;

public:
    pull_coroutine_base( coroutine_context::ctx_fn fn,
                         stack_context * stack_ctx,
                         bool unwind, bool preserve_fpu) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( fn, stack_ctx)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    pull_coroutine_base( coroutine_context const& callee,
                         bool unwind, bool preserve_fpu) :
        use_count_( 0),
        flags_( 0),
        except_(),
        caller_(),
        callee_( callee)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    virtual ~pull_coroutine_base()
    {}

    bool force_unwind() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_force_unwind); }

    bool unwind_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_unwind_stack); }

    bool preserve_fpu() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_preserve_fpu); }

    bool is_complete() const BOOST_NOEXCEPT
    { return 0 != ( flags_ & flag_complete); }

    void pull()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to( & caller_);
        param_type * from(
            reinterpret_cast< param_type * >(
                to.ctx->jump(
                    callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        BOOST_ASSERT( from->ctx);
        callee_ = * from->ctx;
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }

    friend inline void intrusive_ptr_add_ref( pull_coroutine_base * p) BOOST_NOEXCEPT
    { ++p->use_count_; }

    friend inline void intrusive_ptr_release( pull_coroutine_base * p) BOOST_NOEXCEPT
    { if ( --p->use_count_ == 0) p->deallocate_object(); }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_PULL_COROUTINE_BASE_H
