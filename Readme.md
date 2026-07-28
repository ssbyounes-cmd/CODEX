*This project has been created as part of the 42 curriculum by ychoucho.*

## Description

**Codexion** is a POSIX-threads concurrency simulation. A configurable number of *coders* sit around a circular co-working hub and cycle endlessly through three states — **compiling**, **debugging**, and **refactoring**. In the middle of the hub sits a set of shared **USB dongles**: one dongle between every pair of neighboring coders. Compiling requires holding *both* neighboring dongles at once (one in each hand), so a coder can only compile when the two coders sitting next to them are not currently compiling.

Each coder must compile again before `time_to_burnout` milliseconds elapse since their last compile (or since the start of the simulation), or they **burn out**. The simulation is a race against the clock: the program must guarantee that dongles are shared fairly and efficiently enough that no coder is ever starved into burning out, while also correctly detecting and reporting a burnout the moment it happens, should it occur.

The simulation ends in one of two ways:
- **Success** — every coder has compiled at least `number_of_compiles_required` times.
- **Failure** — any single coder burns out.

This is a variation of the classic **Dining Philosophers** problem, extended with a scheduler contract (`fifo` / `edf`), a dongle cooldown period, and strict real-time logging constraints.

## Instructions

### Build

The project is written in C and compiles with `cc` under `-Wall -Wextra -Werror -pthread`.

```sh
make          # builds the codexion binary
make clean    # removes object files
make fclean   # removes object files and the binary
make re       # fclean + all
```

### Run

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coder threads (and number of dongles) sitting in the circle. |
| `time_to_burnout` | Milliseconds a coder can go without starting a compile before burning out. |
| `time_to_compile` | Milliseconds spent holding both dongles while compiling. |
| `time_to_debug` | Milliseconds spent debugging (no dongles held). |
| `time_to_refactor` | Milliseconds spent refactoring (no dongles held); a new compile attempt starts right after. |
| `number_of_compiles_required` | The simulation stops successfully once every coder has reached this many compiles. |
| `dongle_cooldown` | Milliseconds a dongle stays unavailable after being released, before it can be taken again. |
| `scheduler` | Either `fifo` (arrival order) or `edf` (earliest burnout deadline first). |

All eight arguments are mandatory and strictly validated: negative numbers, non-numeric values, overflowing integers, and any `scheduler` value other than `fifo`/`edf` are rejected with an explicit error message and a non-zero exit status.

**Example:**
```sh
./codexion 5 800 200 100 100 5 50 fifo
```

Every state change is logged as:
```
timestamp_in_ms coder_id has taken a dongle
timestamp_in_ms coder_id is compiling
timestamp_in_ms coder_id is debugging
timestamp_in_ms coder_id is refactoring
timestamp_in_ms coder_id burned out
```

## Project structure

| File | Responsibility |
|---|---|
| `main.c` | Entry point: argument count check, parsing, initialization, run, cleanup. |
| `parse.c` | Converts and validates the 8 CLI arguments (custom `atoi` with overflow/sign/whitespace handling, range checks, scheduler name check). |
| `init_sim.c` | Allocates and zero-initializes every dongle, coder slot, and mutex/condition variable, tracking exactly which ones were successfully initialized (for safe teardown on partial failure). |
| `start_sim.c` | Assigns each coder its two neighboring dongles, spawns one thread per coder plus the monitor thread, and joins them all at the end. |
| `routine.c` | The coder's life cycle: take both dongles → compile → release both dongles → debug → refactor → repeat, until `number_of_compiles_required` is reached or the simulation is stopped. |
| `actions.c` | Dongle-level logic: queueing for a dongle, waiting for one's turn (`wait_dongle`), releasing a dongle, and enforcing the cooldown period. |
| `monitor.c` | Independent watchdog thread: continuously checks every coder for burnout and for the global win condition, and triggers a clean, synchronized shutdown either way. |
| `cleanup.c` | Frees every allocation and destroys only the mutexes/condition variables that were actually initialized. |
| `utils.c` | Shared helpers: elapsed-time computation, thread-safe logging, interruptible sleeping, small struct swap. |
| `codexion.h` | All shared types (`t_simulation_data`, `t_coder_info`, `t_dongle`, `t_thread_data`, `t_data_alloc`) and function prototypes. No global variables are used anywhere: every piece of state is threaded through these structs. |

## Blocking cases handled

- **Deadlock prevention (breaking Coffman's Circular Wait):** a deadlock needs all four Coffman conditions at once — Mutual Exclusion, Hold-and-Wait, No Preemption, and Circular Wait. The first three are unavoidable here (a dongle can only be held by one coder at a time, a coder holds one dongle while waiting for the other, and dongles are never forcibly taken back), so the implementation deliberately breaks **Circular Wait**. Every coder normally reaches for their "left" dongle first and their "right" dongle second, but coders with an even id acquire in the opposite order (right, then left) before their first attempt. This is the classic asymmetric Dining Philosophers fix: it guarantees that somewhere around the circle two neighbors reach for the *same* dongle first instead of each waiting on the other's, so a full cycle of mutual waiting can never form, regardless of whether `number_of_coders` is even or odd.
- **Starvation prevention:** because dongles sit strictly between two neighbors, any single dongle can only ever be contested by those two coders — never more. Each dongle keeps a small ordered 2-slot waiting list that is re-ordered on every new request according to the active `scheduler`: `fifo` keeps strict arrival order, while `edf` reorders the two waiting coders by their burnout deadline (`last_compile_start + time_to_burnout`), with deterministic tie-breaks (fewer completed compiles first, then lower coder id) so the policy never stalls on a tie. This guarantees liveness: under `edf`, a coder approaching burnout is never left behind a neighbor with more slack, so no coder starves for dongles as long as the given parameters are feasible.
- **Cooldown handling:** a released dongle records its release timestamp. The next coder allowed to take it is made to sleep out the remainder of `dongle_cooldown` before the dongle is actually granted, so a dongle can never be re-taken before its cooldown has fully elapsed.
- **Precise burnout detection:** a dedicated monitor thread polls every coder's `last_compilation_time` on a tight 1 ms loop and compares it against `time_to_burnout`. The moment a coder is late, the monitor logs the burnout and stops the simulation immediately — well inside the required 10 ms window — instead of waiting for the coder threads themselves to notice.
- **Deterministic, race-free shutdown:** the simulation can end from two different threads (any coder finishing its last compile, or the monitor detecting a burnout). Ending is centralized: a shared `sim_status` flag, protected by its own mutex, is flipped exactly once, and every dongle's condition variable is broadcast so no coder is left permanently blocked in `pthread_cond_wait`. All blocking waits (`wait_dongle`, `safe_sleep`) re-check this flag after waking and exit early, so no thread outlives the simulation or leaks a hung wait.
- **Log serialization:** all `printf` output funnels through a single `print_mutex`, so two state messages can never interleave on the same line or land out of order relative to each other.
- **Memory safety:** initialization tracks each mutex/condition variable's readiness individually (`mutex_ready`, `cond_ready`, `print_ready`, `stop_ready`), so cleanup can run safely and completely even if the program fails partway through setup, without double-destroying or skipping resources. Every `malloc` has a matching `free`.

## Thread synchronization mechanisms

- **`pthread_mutex_t print_mutex`** — serializes every log line so concurrent `is compiling` / `is debugging` / `burned out` messages from different threads never interleave.
- **`pthread_mutex_t stop_mutex`** — protects the shared `sim_status` flag, the single source of truth for "is the simulation still running?". Both the monitor thread and every coder thread read or write it exclusively through `sim_status()`, so the decision to stop is never based on a torn or stale read.
- **One `pthread_mutex_t` + `pthread_cond_t` per dongle** — each dongle's `in_use` flag, 2-slot waiting queue, and `last_used_time` are only ever touched while holding that dongle's mutex. A coder wanting the dongle registers itself in the queue, then calls `pthread_cond_wait` in a loop that re-checks two things on every wake-up: whether the dongle is free, *and* whether it is now this coder's turn according to the scheduler ordering at the front of the queue. This prevents the classic race where two coders both see `in_use == 0` and both grab the dongle: only the mutex-protected check-and-set (`in_use = 1`) inside the same critical section actually hands the dongle over. Releasing a dongle records the timestamp and calls `pthread_cond_broadcast`, waking every waiter so they can re-check the (possibly changed) queue order rather than racing on a single `signal`.
- **One `pthread_mutex_t state_mutex` per coder** — protects `last_compilation_time` and `compile_count`. This is the direct answer to the "thread-safe communication between coders and the monitor" requirement: a coder thread writes `last_compilation_time` under this lock the instant it starts compiling, and the monitor thread reads it under the *same* lock on every 1 ms sweep to test for burnout. Without this mutex, the monitor could read a half-written 64-bit value (a classic word-tearing race) and either miss a burnout or report a false one; with it, every read the monitor performs is guaranteed to see a fully-committed, up-to-date value.
- **No busy-waiting on the hot path:** coders block in `pthread_cond_wait` while genuinely waiting for a dongle, rather than spinning on a lock. The only polling loops in the program are the monitor's 1 ms sweep (needed to bound burnout-detection latency to under 10 ms) and the interruptible sleeps in `safe_sleep`, both of which exist specifically so a burnout or victory signaled by another thread can be noticed quickly instead of only at the end of a long `usleep`.

## Resources

- **[The Dining Philosophers Problem (Wikipedia)](https://en.wikipedia.org/wiki/Dining_philosophers_problem)** — the canonical problem this simulation is modeled on, including the classic asymmetric solution used here to break circular wait.
- **[Coffman's Conditions for Deadlock](https://en.wikipedia.org/wiki/Deadlock#Conditions)** — the four conditions (mutual exclusion, hold and wait, no preemption, circular wait) referenced in the deadlock-prevention design above.
- **[Earliest Deadline First scheduling (Wikipedia)](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)** — the dynamic-priority scheduling algorithm behind the `edf` policy.
- **[POSIX Threads Programming (LLNL tutorial)](https://hpc-tutorials.llnl.gov/posix/)** — reference for `pthread_create`/`pthread_join`, mutexes, and condition variables.
- **[`pthread_cond_wait`/`pthread_cond_timedwait` man page](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3.html)** — details on the wait/broadcast pattern and spurious wake-up handling used throughout `actions.c` and `monitor.c`.
- **[`gettimeofday(2)` man page](https://man7.org/linux/man-pages/man2/gettimeofday.2.html)** — reference for the millisecond timestamps used for logging and burnout detection.
- **AI usage:** AI assistance was used to help understand core `pthread` concepts (mutex/condition-variable semantics, spurious wake-ups, the producer/consumer wait pattern), to reason through edge cases of the asymmetric deadlock-avoidance strategy for both even and odd numbers of coders, and to help structure and word this README.
