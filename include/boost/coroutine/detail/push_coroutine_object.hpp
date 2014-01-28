
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_PUSH_COROUTINE_OBJECT_H
#define BOOST_COROUTINES_DETAIL_PUSH_COROUTINE_OBJECT_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/utility.hpp>

#include <boost/coroutine/attributes.hpp>
#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/coroutine/detail/flags.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/flags.hpp>
#include <boost/coroutine/detail/push_coroutine_base.hpp>

#ifdef BOOST_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4355) // using 'this' in initializer list
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace coroutines {
namespace detail {

template< typename Arg, typename Fn, typename Caller >
class push_coroutine_object : public push_coroutine_base< Arg >
{
private:
    typedef push_coroutine_base< Arg >                  base_type;
    typedef parameters< Arg >                           param_type;

    Fn                      fn_;

    push_coroutine_object( push_coroutine_object &);
    push_coroutine_object & operator=( push_coroutine_object const&);

public:
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
    push_coroutine_object( Fn fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
        fn_( fn)
    {}
#endif
    push_coroutine_object( BOOST_RV_REF( Fn) fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
        fn_( fn)
#else
        fn_( forward< Fn >( fn) )
#endif
    {}

    void run()
    {
        {
            param_type * from(
                reinterpret_cast< param_type * >(
                    this->callee_->jump(
                        * this->caller_,
                        reinterpret_cast< intptr_t >( this),
                        this->preserve_fpu() ) ) );
            BOOST_ASSERT( from->data);

            // create pull_coroutine
            typename Caller::base_t b( this->callee_, this->caller_, false, this->preserve_fpu(), from->data);
            Caller c( & b);
            try
            { fn_( c); }
            catch ( forced_unwind const&)
            {}
            catch (...)
            { this->except_ = current_exception(); }
        }

        this->flags_ |= flag_complete;
        param_type to;
        this->callee_->jump(
            * this->caller_,
            reinterpret_cast< intptr_t >( & to),
            this->preserve_fpu() );
        BOOST_ASSERT_MSG( false, "push_coroutine is complete");
    }
};

template< typename Arg, typename Fn, typename Caller >
class push_coroutine_object< Arg &, Fn, Caller > : public push_coroutine_base< Arg & >
{
private:
    typedef push_coroutine_base< Arg & >                base_type;
    typedef parameters< Arg & >                         param_type;

    Fn                      fn_;

    push_coroutine_object( push_coroutine_object &);
    push_coroutine_object & operator=( push_coroutine_object const&);

public:
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    push_coroutine_object( BOOST_RV_REF( Fn) fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
        fn_( forward< Fn >( fn) )
    {}
#else
    push_coroutine_object( Fn fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
        fn_( fn)
    {}

    push_coroutine_object( BOOST_RV_REF( Fn) fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
        fn_( fn)
    {}
#endif

    void run()
    {
        {
            param_type * from(
                reinterpret_cast< param_type * >(
                    this->callee_->jump(
                        * this->caller_,
                        reinterpret_cast< intptr_t >( this),
                        this->preserve_fpu() ) ) );
            BOOST_ASSERT( from->data);

            // create pull_coroutine
            typename Caller::base_t b( this->callee_, this->caller_, false, this->preserve_fpu(), from->data);
            Caller c( & b);
            try
            { fn_( c); }
            catch ( forced_unwind const&)
            {}
            catch (...)
            { this->except_ = current_exception(); }
        }

        this->flags_ |= flag_complete;
        param_type to;
        this->callee_->jump(
            * this->caller_,
            reinterpret_cast< intptr_t >( & to),
            this->preserve_fpu() );
        BOOST_ASSERT_MSG( false, "push_coroutine is complete");
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#ifdef BOOST_MSVC
 #pragma warning (pop)
#endif

#endif // BOOST_COROUTINES_DETAIL_PUSH_COROUTINE_OBJECT_H
