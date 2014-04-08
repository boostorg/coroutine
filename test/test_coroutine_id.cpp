#include <boost/test/unit_test.hpp>
#include <boost/coroutine/coroutine_id.hpp>
#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <boost/static_assert.hpp>
#include <boost/type_traits/has_less.hpp>

#include <boost/thread/thread.hpp>

namespace coro = boost::coroutines;
namespace this_coro = coro::this_coroutine;

BOOST_STATIC_ASSERT_MSG(
        boost::has_less<this_coro::coroutine_id>::value,
        "coroutine_id should be less than comparable");

void get_id_should_return_bool_convertible_false_\
if_called_before_any_context_switch()
{
    BOOST_CHECK(!this_coro::get_id());
}

void get_id_should_not_throw_outside_a_coroutine()
{
    BOOST_CHECK_NO_THROW(this_coro::get_id());
}


void f1( coro::asymmetric_coroutine< void >::push_type &)
{
    BOOST_CHECK_NO_THROW(this_coro::get_id());
}
void get_id_should_not_throw_inside_a_coroutine()
{
    coro::asymmetric_coroutine< void >::pull_type coro(f1);
}


void f2( coro::asymmetric_coroutine< void >::push_type & push)
{
    this_coro::coroutine_id id1 = this_coro::get_id();
    push();
    this_coro::coroutine_id id2 = this_coro::get_id();
    push();
    this_coro::coroutine_id id3 = this_coro::get_id();
    push();
    BOOST_CHECK_EQUAL(id1, id2);
    BOOST_CHECK_EQUAL(id2, id3);
}
void id_should_remain_the_same_during_the_whole_coroutine_function()
{
    coro::asymmetric_coroutine< void >::pull_type coro(f2);
    coro();
    coro();
    coro();
    BOOST_CHECK( ! coro);
}


void f3( coro::asymmetric_coroutine< void >::push_type & push)
{
    this_coro::coroutine_id id1 = this_coro::get_id();
    push();
    this_coro::coroutine_id id2 = this_coro::get_id();
    push();
    this_coro::coroutine_id id3 = this_coro::get_id();
    push();
    BOOST_CHECK_EQUAL(id1, id2);
    BOOST_CHECK_EQUAL(id2, id3);
}
void id_should_remain_the_same_during_the_whole_coroutine_function\
when_there_are_more_coroutine_functions()
{
    coro::asymmetric_coroutine< void >::pull_type coro1(f3);
    coro::asymmetric_coroutine< void >::pull_type coro2(f3);
    coro1();
    coro2();
    coro1();
    coro2();
    coro1();
    coro2();
    BOOST_CHECK( ! coro1);
    BOOST_CHECK( ! coro2);
}


void f4( coro::asymmetric_coroutine<this_coro::coroutine_id>::push_type & push)
{
    this_coro::coroutine_id id1 = this_coro::get_id();
    push(id1);
    this_coro::coroutine_id id2 = this_coro::get_id();
    push(id2);
    BOOST_CHECK_EQUAL(id1, id2);
}
void id_should_remain_the_same_during_the_whole_coroutine_function\
and_ids_should_be_different_in_different_coroutine_functions()
{
    coro::asymmetric_coroutine<this_coro::coroutine_id>::pull_type coro1(f4);
    coro::asymmetric_coroutine<this_coro::coroutine_id>::pull_type coro2(f4);
    BOOST_CHECK(coro1.get() != coro2.get());
    coro1();
    coro2();
    BOOST_CHECK(coro1.get() != coro2.get());
    coro1();
    coro2();
    BOOST_CHECK( ! coro1);
    BOOST_CHECK( ! coro2);
}


struct thread_id_coro_id {
    boost::thread::id thread_id;
    this_coro::coroutine_id coro_id;
    thread_id_coro_id(boost::thread::id thread_id,
            this_coro::coroutine_id coro_id) : thread_id(thread_id),
                coro_id(coro_id) {}
};
void f5( coro::asymmetric_coroutine<thread_id_coro_id>::push_type & push)
{
    boost::thread::id thread_id = boost::this_thread::get_id();
    this_coro::coroutine_id coro_id1 = this_coro::get_id();
    push(thread_id_coro_id(thread_id, coro_id1));
    thread_id = boost::this_thread::get_id();
    this_coro::coroutine_id coro_id2 = this_coro::get_id();
    push(thread_id_coro_id(thread_id, coro_id2));
    BOOST_CHECK_EQUAL(coro_id1, coro_id2);
}
struct thread_functor{
    coro::asymmetric_coroutine<thread_id_coro_id>::pull_type *coro;
    thread_functor(
        coro::asymmetric_coroutine<thread_id_coro_id>::pull_type* coro) :
            coro(coro) {}
    void operator()() {
        coro->operator()();
    }
};
void id_should_remain_the_same_during_the_whole_coroutine_function\
and_ids_should_be_different_in_different_coroutine_functions\
when_threre_are_more_threads()
{
    coro::asymmetric_coroutine<thread_id_coro_id>::pull_type coro1(f5);
    coro::asymmetric_coroutine<thread_id_coro_id>::pull_type coro2(f5);
    BOOST_CHECK_NE(coro1.get().coro_id, coro2.get().coro_id);
    BOOST_CHECK_EQUAL(coro1.get().thread_id, coro2.get().thread_id);
    thread_functor tf(&coro1);
    boost::thread t(tf);
    coro2();
    t.join();
    BOOST_CHECK_NE(coro1.get().coro_id, coro2.get().coro_id);
    BOOST_CHECK_NE(coro1.get().thread_id, coro2.get().thread_id);
    coro1();
    coro2();
    BOOST_CHECK( ! coro1);
    BOOST_CHECK( ! coro2);
}


boost::unit_test::test_suite * init_unit_test_suite( int, char* [])
{
    boost::unit_test::test_suite * test =
        BOOST_TEST_SUITE("Boost.coroutine: coroutine id test suite");

    test->add( BOOST_TEST_CASE( &get_id_should_return_bool_convertible_false_\
if_called_before_any_context_switch) );

    test->add( BOOST_TEST_CASE( &get_id_should_not_throw_outside_a_coroutine));

    test->add( BOOST_TEST_CASE( &get_id_should_not_throw_inside_a_coroutine));

    test->add( BOOST_TEST_CASE(
        &id_should_remain_the_same_during_the_whole_coroutine_function));

    test->add( BOOST_TEST_CASE(
        &id_should_remain_the_same_during_the_whole_coroutine_function\
when_there_are_more_coroutine_functions));

    test->add( BOOST_TEST_CASE(
        &id_should_remain_the_same_during_the_whole_coroutine_function\
and_ids_should_be_different_in_different_coroutine_functions));

    test->add( BOOST_TEST_CASE(
        &id_should_remain_the_same_during_the_whole_coroutine_function\
and_ids_should_be_different_in_different_coroutine_functions\
when_threre_are_more_threads));

    return test;
}
