#include "mp_realtime_scheduler.h"

#include <algorithm>

namespace cata_mp
{

mp_realtime_scheduler::mp_realtime_scheduler( duration tick_interval,
        std::uint32_t max_catch_up_ticks )
    : tick_interval_( tick_interval > duration::zero() ? tick_interval :
                      std::chrono::milliseconds( 100 ) ),
      max_catch_up_ticks_( std::max<std::uint32_t>( 1, max_catch_up_ticks ) )
{
}

void mp_realtime_scheduler::reset( time_point now )
{
    accumulated_ = duration::zero();
    last_update_ = now;
    has_time_baseline_ = true;
}

std::uint32_t mp_realtime_scheduler::advance( time_point now )
{
    if( !has_time_baseline_ ) {
        reset( now );
        return 0;
    }

    if( now <= last_update_ ) {
        last_update_ = now;
        return 0;
    }

    const duration elapsed = now - last_update_;
    last_update_ = now;
    return consume_elapsed( elapsed );
}

std::uint32_t mp_realtime_scheduler::consume_elapsed( duration elapsed )
{
    if( elapsed > duration::zero() ) {
        accumulated_ += elapsed;
    }

    const auto due = accumulated_ / tick_interval_;
    const auto ticks = static_cast<std::uint32_t>(
                           std::min<duration::rep>( due, max_catch_up_ticks_ ) );
    accumulated_ -= tick_interval_ * ticks;
    return ticks;
}

void mp_realtime_scheduler::set_tick_interval( duration interval )
{
    if( interval <= duration::zero() ) {
        return;
    }
    tick_interval_ = interval;
}

void mp_realtime_scheduler::set_max_catch_up_ticks( std::uint32_t maximum )
{
    max_catch_up_ticks_ = std::max<std::uint32_t>( 1, maximum );
}

mp_realtime_scheduler::duration mp_realtime_scheduler::tick_interval() const
{
    return tick_interval_;
}

mp_realtime_scheduler::duration mp_realtime_scheduler::accumulated_time() const
{
    return accumulated_;
}

float mp_realtime_scheduler::interpolation_alpha() const
{
    const double alpha = std::chrono::duration<double>( accumulated_ ).count() /
                         std::chrono::duration<double>( tick_interval_ ).count();
    return static_cast<float>( std::clamp( alpha, 0.0, 1.0 ) );
}

} // namespace cata_mp
