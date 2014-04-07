#ifndef BOOST_COROUTINES_COROUTINE_ID_HPP
#define BOOST_COROUTINES_COROUTINE_ID_HPP

#include <boost/config.hpp>
#include <boost/context/fcontext.hpp>

namespace boost{
namespace coroutines {
namespace this_coroutine {

typedef const context::fcontext_t* coroutine_id;
coroutine_id get_id();

}}}


#endif /* BOOST_COROUTINES_COROUTINE_ID_HPP */
