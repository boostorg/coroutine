
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_PULL_COROUTINE_BASE_H
#define BOOST_COROUTINES_DETAIL_PULL_COROUTINE_BASE_H

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility.hpp>

#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/coroutine/detail/flags.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/detail/trampoline.hpp>
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
private:
    template<
        typename X, typename Y, typename Z
    >
    friend class push_coroutine_object;

    typedef parameters< R >                           param_type;

protected:
    int                     flags_;
    exception_ptr           except_;
    coroutine_context   *   caller_;
    coroutine_context   *   callee_;
    R                   *   result_;

public:
    pull_coroutine_base( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu) :
        flags_( 0),
        except_(),
        caller_( caller),
        callee_( callee),
        result_( 0)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    pull_coroutine_base( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu,
                         R * result) :
        flags_( 0),
        except_(),
        caller_( caller),
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

    void unwind_stack() BOOST_NOEXCEPT
    {
        if ( ! is_complete() && force_unwind() )
        {
            flags_ |= flag_unwind_stack;
            param_type to( unwind_t::force_unwind);
            caller_->jump(
                * callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void pull()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        param_type * from(
            reinterpret_cast< param_type * >(
                caller_->jump(
                    * callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
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
};

template< typename R >
class pull_coroutine_base< R & > : private noncopyable
{
private:
    template<
        typename X, typename Y, typename Z
    >
    friend class push_coroutine_object;

    typedef parameters< R & >                           param_type;

protected:
    int                     flags_;
    exception_ptr           except_;
    coroutine_context   *   caller_;
    coroutine_context   *   callee_;
    R                   *   result_;

public:
    pull_coroutine_base( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu) :
        flags_( 0),
        except_(),
        caller_( caller),
        callee_( callee),
        result_( 0)
    {
        if ( unwind) flags_ |= flag_force_unwind;
        if ( preserve_fpu) flags_ |= flag_preserve_fpu;
    }

    pull_coroutine_base( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu,
                         R * result) :
        flags_( 0),
        except_(),
        caller_( caller),
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

    void unwind_stack() BOOST_NOEXCEPT
    {
        if ( ! is_complete() && force_unwind() )
        {
            flags_ |= flag_unwind_stack;
            param_type to( unwind_t::force_unwind);
            caller_->jump(
                * callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void pull()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        param_type * from(
            reinterpret_cast< param_type * >(
                caller_->jump(
                    * callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
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
};

template<>
class pull_coroutine_base< void > : private noncopyable
{
private:
    template< typename X, typename Y, typename Z >
    friend class push_coroutine_object;
    template<
        typename X, typename Y, typename Z
    >
    friend void trampoline_void( intptr_t);

    typedef parameters< void >      param_type;

    int                     flags_;
    exception_ptr           except_;
    coroutine_context   *   caller_;
    coroutine_context   *   callee_;

public:
    pull_coroutine_base( coroutine_context * caller,
                         coroutine_context * callee,
                         bool unwind, bool preserve_fpu) :
        flags_( 0),
        except_(),
        caller_( caller),
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

    void unwind_stack() BOOST_NOEXCEPT
    {
        if ( ! is_complete() && force_unwind() )
        {
            flags_ |= flag_unwind_stack;
            param_type to( unwind_t::force_unwind);
            caller_->jump(
                * callee_,
                reinterpret_cast< intptr_t >( & to),
                preserve_fpu() );
            flags_ &= ~flag_unwind_stack;

            BOOST_ASSERT( is_complete() );
        }
    }

    void pull()
    {
        BOOST_ASSERT( ! is_complete() );

        param_type to;
        param_type * from(
            reinterpret_cast< param_type * >(
                caller_->jump(
                    * callee_,
                    reinterpret_cast< intptr_t >( & to),
                    preserve_fpu() ) ) );
        if ( from->do_unwind) throw forced_unwind();
        if ( except_) rethrow_exception( except_);
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_COROUTINES_DETAIL_PULL_COROUTINE_BASE_H
