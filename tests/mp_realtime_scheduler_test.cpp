#include <chrono>

#include "cata_catch.h"
#include "mp_realtime_scheduler.h"

using cata_mp::mp_realtime_scheduler;

TEST_CASE( "real_time_scheduler_uses_fixed_ticks", "[multiplayer][scheduler][nogame]" )
{
    mp_realtime_scheduler scheduler( std::chrono::milliseconds( 100 ), 8 );

    CHECK( scheduler.consume_elapsed( std::chrono::milliseconds( 99 ) ) == 0 );
    CHECK( scheduler.accumulated_time() == std::chrono::milliseconds( 99 ) );
    CHECK( scheduler.consume_elapsed( std::chrono::milliseconds( 1 ) ) == 1 );
    CHECK( scheduler.accumulated_time() == std::chrono::steady_clock::duration::zero() );
}

TEST_CASE( "real_time_scheduler_caps_catch_up_without_dropping_time",
           "[multiplayer][scheduler][nogame]" )
{
    mp_realtime_scheduler scheduler( std::chrono::milliseconds( 100 ), 3 );

    CHECK( scheduler.consume_elapsed( std::chrono::milliseconds( 850 ) ) == 3 );
    CHECK( scheduler.accumulated_time() == std::chrono::milliseconds( 550 ) );
    CHECK( scheduler.consume_elapsed( std::chrono::milliseconds( 0 ) ) == 3 );
    CHECK( scheduler.accumulated_time() == std::chrono::milliseconds( 250 ) );
    CHECK( scheduler.consume_elapsed( std::chrono::milliseconds( 0 ) ) == 2 );
    CHECK( scheduler.accumulated_time() == std::chrono::milliseconds( 50 ) );
}

TEST_CASE( "real_time_scheduler_interpolation_is_bounded",
           "[multiplayer][scheduler][nogame]" )
{
    mp_realtime_scheduler scheduler( std::chrono::milliseconds( 100 ), 8 );

    scheduler.consume_elapsed( std::chrono::milliseconds( 25 ) );
    CHECK( scheduler.interpolation_alpha() == Catch::Approx( 0.25f ) );
    scheduler.consume_elapsed( std::chrono::milliseconds( 200 ) );
    CHECK( scheduler.interpolation_alpha() == Catch::Approx( 0.25f ) );
}

TEST_CASE( "real_time_scheduler_uses_monotonic_baseline",
           "[multiplayer][scheduler][nogame]" )
{
    using clock = mp_realtime_scheduler::clock;
    const clock::time_point start{};
    mp_realtime_scheduler scheduler( std::chrono::milliseconds( 100 ), 8 );

    CHECK( scheduler.advance( start ) == 0 );
    CHECK( scheduler.advance( start + std::chrono::milliseconds( 99 ) ) == 0 );
    CHECK( scheduler.advance( start + std::chrono::milliseconds( 100 ) ) == 1 );
    CHECK( scheduler.advance( start + std::chrono::milliseconds( 90 ) ) == 0 );
    CHECK( scheduler.advance( start + std::chrono::milliseconds( 200 ) ) == 1 );
}

TEST_CASE( "real_time_scheduler_rejects_invalid_configuration",
           "[multiplayer][scheduler][nogame]" )
{
    mp_realtime_scheduler scheduler( std::chrono::milliseconds( 0 ), 0 );

    CHECK( scheduler.tick_interval() == std::chrono::milliseconds( 100 ) );
    scheduler.set_tick_interval( std::chrono::milliseconds( 0 ) );
    CHECK( scheduler.tick_interval() == std::chrono::milliseconds( 100 ) );

    scheduler.set_max_catch_up_ticks( 0 );
    CHECK( scheduler.consume_elapsed( std::chrono::milliseconds( 500 ) ) == 1 );
}
