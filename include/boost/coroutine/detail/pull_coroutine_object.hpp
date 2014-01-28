
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COROUTINES_DETAIL_PULL_COROUTINE_OBJECT_H
#define BOOST_COROUTINES_DETAIL_PULL_COROUTINE_OBJECT_H

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/move/move.hpp>

#include <boost/coroutine/attributes.hpp>
#include <boost/coroutine/detail/config.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/coroutine/detail/flags.hpp>
#include <boost/coroutine/detail/parameters.hpp>
#include <boost/coroutine/flags.hpp>
#include <boost/coroutine/detail/pull_coroutine_base.hpp>

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

template< typename R, typename Fn, typename Caller >
class pull_coroutine_object : public pull_coroutine_base< R >
{
private:
    typedef pull_coroutine_base< R >                    base_type;
    typedef parameters< R >                             param_type;

    Fn                      fn_;

    pull_coroutine_object( pull_coroutine_object &);
    pull_coroutine_object & operator=( pull_coroutine_object const&);

public:
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
    pull_coroutine_object( Fn fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
        fn_( fn)
    {}
#endif
    pull_coroutine_object( BOOST_RV_REF( Fn) fn, attributes const& attr,
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
            this->result_ = from->data;

            // create push_coroutine
            typename Caller::base_t b( this->callee_, this->caller_, false, this->preserve_fpu() );
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
        BOOST_ASSERT_MSG( false, "pull_coroutine is complete");
    }
};

template< typename R, typename Fn, typename Caller >
class pull_coroutine_object< R &, Fn, Caller > : public pull_coroutine_base< R & >
{
private:
    typedef pull_coroutine_base< R & >                  base_type;
    typedef parameters< R & >                           param_type;

    Fn                      fn_;

    pull_coroutine_object( pull_coroutine_object &);
    pull_coroutine_object & operator=( pull_coroutine_object const&);

public:
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    pull_coroutine_object( BOOST_RV_REF( Fn) fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
        fn_( forward< Fn >( fn) )
    {}
#else
    pull_coroutine_object( Fn fn, attributes const& attr,
                           coroutine_context * caller,
                           coroutine_context * callee) :
        base_type( caller, callee,
                   stack_unwind == attr.do_unwind,
                   fpu_preserved == attr.preserve_fpu),
        fn_( fn)
    {}

    pull_coroutine_object( BOOST_RV_REF( Fn) fn, attributes const& attr,
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
            this->result_ = from->data;

            // create push_coroutine
            typename Caller::base_t b( this->callee_, this->caller_, false, this->preserve_fpu() );
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
        BOOST_ASSERT_MSG( false, "pull_coroutine is complete");
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#ifdef BOOST_MSVC
 #pragma warning (pop)
#endif

#endif // BOOST_COROUTINES_DETAIL_PULL_COROUTINE_OBJECT_H
