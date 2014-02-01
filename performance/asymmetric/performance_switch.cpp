
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/chrono.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>

#include "../bind_processor.hpp"
#include "../clock.hpp"
#include "../cycle.hpp"

boost::coroutines::flag_fpu_t preserve_fpu = boost::coroutines::fpu_not_preserved;
boost::uint64_t jobs = 1000;

struct X
{
    std::string str;

    X( std::string const& str_) :
        str( str_)
    {}
};

const X x("abc");

void fn_void( boost::coroutines::coroutine< void >::push_type & c)
{ while ( true) c(); }

void fn_int( boost::coroutines::coroutine< int >::push_type & c)
{ while ( true) c( 7); }

void fn_x( boost::coroutines::coroutine< X >::push_type & c)
{
    while ( true) c( x);
}

duration_type measure_time_void()
{
    boost::coroutines::coroutine< void >::pull_type c( fn_void, boost::coroutines::attributes( preserve_fpu) );

    // cache warum-up
    c();
        
    time_point_type start( clock_type::now() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        c();
    }
    duration_type total = clock_type::now() - start;
    total -= overhead_clock(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

duration_type measure_time_int()
{
    boost::coroutines::coroutine< int >::pull_type c( fn_int, boost::coroutines::attributes( preserve_fpu) );

    // cache warum-up
    c();
        
    time_point_type start( clock_type::now() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        c();
    }
    duration_type total = clock_type::now() - start;
    total -= overhead_clock(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

duration_type measure_time_x()
{
    boost::coroutines::coroutine< X >::pull_type c( fn_x, boost::coroutines::attributes( preserve_fpu) );

    // cache warum-up
    c();
        
    time_point_type start( clock_type::now() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        c();
    }
    duration_type total = clock_type::now() - start;
    total -= overhead_clock(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

# ifdef BOOST_CONTEXT_CYCLE
cycle_type measure_cycles_void()
{
    boost::coroutines::coroutine< void >::pull_type c( fn_void, boost::coroutines::attributes( preserve_fpu) );

    // cache warum-up
    c();
        
    cycle_type start( cycles() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        c();
    }
    cycle_type total = cycles() - start;
    total -= overhead_cycle(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

cycle_type measure_cycles_int()
{
    boost::coroutines::coroutine< int >::pull_type c( fn_int, boost::coroutines::attributes( preserve_fpu) );

    // cache warum-up
    c();
        
    cycle_type start( cycles() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        c();
    }
    cycle_type total = cycles() - start;
    total -= overhead_cycle(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

cycle_type measure_cycles_x()
{
    boost::coroutines::coroutine< X >::pull_type c( fn_x, boost::coroutines::attributes( preserve_fpu) );

    // cache warum-up
    c();
        
    cycle_type start( cycles() );
    for ( std::size_t i = 0; i < jobs; ++i) {
        c();
    }
    cycle_type total = cycles() - start;
    total -= overhead_cycle(); // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}
# endif

int main( int argc, char * argv[])
{
    try
    {
        bind_to_processor( 0);

        bool preserve = false;
        boost::program_options::options_description desc("allowed options");
        desc.add_options()
            ("help", "help message")
            ("fpu,f", boost::program_options::value< bool >( & preserve), "preserve FPU registers")
            ("jobs,j", boost::program_options::value< boost::uint64_t >( & jobs), "jobs to run");

        boost::program_options::variables_map vm;
        boost::program_options::store(
                boost::program_options::parse_command_line(
                    argc,
                    argv,
                    desc),
                vm);
        boost::program_options::notify( vm);

        if ( vm.count("help") ) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        if ( preserve) preserve_fpu = boost::coroutines::fpu_preserved;

        boost::uint64_t res = measure_time_void().count();
        std::cout << "void: average of " << res << " nano seconds" << std::endl;
        res = measure_time_int().count();
        std::cout << "int: average of " << res << " nano seconds" << std::endl;
        res = measure_time_x().count();
        std::cout << "X: average of " << res << " nano seconds" << std::endl;
#ifdef BOOST_CONTEXT_CYCLE
        res = measure_cycles_void();
        std::cout << "void: average of " << res << " cpu cycles" << std::endl;
        res = measure_cycles_int();
        std::cout << "int: average of " << res << " cpu cycles" << std::endl;
        res = measure_cycles_x();
        std::cout << "X: average of " << res << " cpu cycles" << std::endl;
#endif

        return EXIT_SUCCESS;
    }
    catch ( std::exception const& e)
    { std::cerr << "exception: " << e.what() << std::endl; }
    catch (...)
    { std::cerr << "unhandled exception" << std::endl; }
    return EXIT_FAILURE;
}
