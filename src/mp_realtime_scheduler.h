#pragma once
#ifndef CATA_SRC_MP_REALTIME_SCHEDULER_H
#define CATA_SRC_MP_REALTIME_SCHEDULER_H

#include <chrono>
#include <cstdint>

namespace cata_mp
{

// Fixed-step wall-clock scheduler for the optional real-time simulation.
//
// The scheduler deliberately does not run game logic itself.  The game loop
// asks how many simulation ticks are due, then runs the authoritative world
// update once for each returned tick.  If the machine falls behind, unconsumed
// time stays in the accumulator instead of being silently discarded.
class mp_realtime_scheduler
{
    public:
        using clock = std::chrono::steady_clock;
        using duration = clock::duration;
        using time_point = clock::time_point;

        explicit mp_realtime_scheduler(
            duration tick_interval = std::chrono::milliseconds( 100 ),
            std::uint32_t max_catch_up_ticks = 8 );

        // Start or restart the wall-clock baseline and clear accumulated time.
        void reset( time_point now = clock::now() );

        // Add elapsed wall-clock time and return the number of fixed simulation
        // ticks that can be processed now.  The result is capped per call;
        // remaining whole ticks stay queued for subsequent calls.
        std::uint32_t advance( time_point now = clock::now() );

        // Deterministic entry point for tests and callers with an elapsed delta.
        std::uint32_t consume_elapsed( duration elapsed );

        void set_tick_interval( duration interval );
        void set_max_catch_up_ticks( std::uint32_t maximum );

        duration tick_interval() const;
        duration accumulated_time() const;
        float interpolation_alpha() const;

    private:
        duration tick_interval_;
        duration accumulated_{ duration::zero() };
        std::uint32_t max_catch_up_ticks_;
        time_point last_update_{};
        bool has_time_baseline_ = false;
};

} // namespace cata_mp

#endif // CATA_SRC_MP_REALTIME_SCHEDULER_H
