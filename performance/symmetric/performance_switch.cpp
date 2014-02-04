
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
time_point_type end;

struct X
{
    std::string str;

    X( std::string const& str_) :
        str( str_)
    {}
};

const X x("abc");

void fn_void( boost::coroutines::symmetric_coroutine< void >::self_type &)
{}

void fn_int( boost::coroutines::symmetric_coroutine< int >::self_type &)
{}

void fn_x( boost::coroutines::symmetric_coroutine< X >::self_type &)
{}

duration_type measure_time_void( duration_type overhead)
{
    duration_type total = duration_type::zero();
    for ( std::size_t i = 0; i < jobs; ++i) {
        boost::coroutines::symmetric_coroutine< void > c( fn_void,
                boost::coroutines::attributes( preserve_fpu) );
        time_point_type start( clock_type::now() );
        c();
        total += clock_type::now() - start;
    }
    total -= jobs * overhead; // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

duration_type measure_time_int( duration_type overhead)
{
    int i = 3;
    duration_type total = duration_type::zero();
    for ( std::size_t i = 0; i < jobs; ++i) {
        boost::coroutines::symmetric_coroutine< int > c( fn_int,
                boost::coroutines::attributes( preserve_fpu) );
        time_point_type start( clock_type::now() );
        c( i);
        total += clock_type::now() - start;
    }
    total -= jobs * overhead; // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

duration_type measure_time_x( duration_type overhead)
{
    X x("abc");
    duration_type total = duration_type::zero();
    for ( std::size_t i = 0; i < jobs; ++i) {
        boost::coroutines::symmetric_coroutine< X > c( fn_x,
                boost::coroutines::attributes( preserve_fpu) );
        time_point_type start( clock_type::now() );
        c( x);
        total += clock_type::now() - start;
    }
    total -= jobs * overhead; // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

# ifdef BOOST_CONTEXT_CYCLE
cycle_type measure_cycles_void( cycle_type overhead)
{
    cycle_type total = 0;
    for ( std::size_t i = 0; i < jobs; ++i) {
        boost::coroutines::symmetric_coroutine< void > c( fn_void,
            boost::coroutines::attributes( preserve_fpu) );
        cycle_type start( cycles() );
        c();
        total += cycles() - start;
    }
    total -= jobs * overhead; // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

cycle_type measure_cycles_int( cycle_type overhead)
{
    int i = 3;
    cycle_type total = 0;
    for ( std::size_t i = 0; i < jobs; ++i) {
        boost::coroutines::symmetric_coroutine< int > c( fn_int,
            boost::coroutines::attributes( preserve_fpu) );
        cycle_type start( cycles() );
        c( i);
        total += cycles() - start;
    }
    total -= jobs * overhead; // overhead of measurement
    total /= jobs;  // loops
    total /= 2;  // 2x jump_fcontext

    return total;
}

cycle_type measure_cycles_x( cycle_type overhead)
{
    X x("abc");
    cycle_type total = 0;
    for ( std::size_t i = 0; i < jobs; ++i) {
        boost::coroutines::symmetric_coroutine< X > c( fn_x,
            boost::coroutines::attributes( preserve_fpu) );
        cycle_type start( cycles() );
        c( x);
        total += cycles() - start;
    }
    total -= jobs * overhead; // overhead of measurement
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

        duration_type overhead_c = overhead_clock();
        std::cout << "overhead " << overhead_c.count() << " nano seconds" << std::endl;
        boost::uint64_t res = measure_time_void( overhead_c).count();
        std::cout << "void: average of " << res << " nano seconds" << std::endl;
        res = measure_time_int( overhead_c).count();
        std::cout << "int: average of " << res << " nano seconds" << std::endl;
        res = measure_time_x( overhead_c).count();
        std::cout << "X: average of " << res << " nano seconds" << std::endl;
#ifdef BOOST_CONTEXT_CYCLE
        cycle_type overhead_y = overhead_cycle();
        std::cout << "overhead " << overhead_y << " cpu cycles" << std::endl;
        res = measure_cycles_void( overhead_y);
        std::cout << "void: average of " << res << " cpu cycles" << std::endl;
        res = measure_cycles_int( overhead_y);
        std::cout << "int: average of " << res << " cpu cycles" << std::endl;
        res = measure_cycles_x( overhead_y);
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
