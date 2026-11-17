# Real-time survival mode implementation notes

The existing co-op simulation is authoritative on the host and uses a lockstep
turn grant / acknowledgement protocol. Real-time mode must not reuse that
blocking protocol as-is: a client waiting for a turn and a host waiting for a
client action are both intentional parts of classic mode.

## Fixed-step scheduler foundation

`mp_realtime_scheduler` is the first isolated component for the alternate
simulation path. It measures elapsed steady-clock time and reports a bounded
number of fixed simulation ticks to the game loop.

Important properties:

- It uses a monotonic clock, not wall-clock calendar time.
- The default tick interval is 100 ms (10 simulation ticks per second).
- A call is capped at eight ticks to avoid one frame monopolizing the game loop.
- Time not processed because of that cap remains accumulated; it is not dropped.
- It has no game-state or network side effects and is only advanced by the game
  thread. The host remains the authority for world simulation.

## Next integration steps

1. Persist a per-world simulation-mode choice, defaulting to classic turn-based.
2. Branch the host game loop so real-time mode advances through this scheduler
   instead of the lockstep grant / wait path.
3. Add timestamped player-intent messages and host-side action validation.
4. Replicate authoritative tick numbers and state snapshots to clients.
5. Keep the current turn-based grant and acknowledgement path unchanged.

The scheduler alone does not enable playable real-time mode. It is deliberately
isolated until the action pipeline and synchronization rules are implemented.
