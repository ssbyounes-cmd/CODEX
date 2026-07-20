1. We implemented the swapping of dongles for "even" coders so we can avoid the possibility of deadlock.

2. We stagger the thread start times (delaying even coders slightly at T=0ms) to guarantee maximum table throughput and prevent the 'roadblock effect'.

3. take_dongle() and release_dongle() are two protected operations that ensure that only one thread (out of the two that shares the dongle) can take or release a dongle at a time, preventing race conditions

- cond_wait() is used to block one thread while another thread is using a dongle, ensuring that threads wait their turn to access shared resources.

4. Print and Stop have both their own mutexes to ensure that the output is not garbled and that the program can be stopped safely without leaving threads in an inconsistent state.

5. safe_sleep() was preferred over usleep() because it allows the thread to be interrupted if the program is stopped, preventing the program from hanging indefinitely.

6. Schedulers edf and fifo are implemented to allow the user to choose between two different scheduling algorithms for the threads, providing flexibility in how the program manages thread execution.

- They were implemented using a priority queue to manage the order of thread execution based on their deadlines or arrival times.

- A coder must register first in both queues of his left and right dongle before he can be scheduled to run. This ensures that the scheduler has all the necessary information to make informed decisions about thread execution order.


7. Coder_info (lastcompilationtime, compile_count) need their own state_mutex since the monitor 
needs to access both in order to detect burn outs or terminate simulation in case all coders compiled.

TBC: -

-Last update was turning safe_sleep and wait dongle returning functions so they
can stop all threads from continuing the simulation




















The Process is the "Container" (Heavy): It is the isolated environment given by the operating system. It owns the memory space, the open files, and the security permissions. If one process crashes, the others keep running fine.

The Thread is the "Worker" (Light): It is the actual sequence of instructions executing inside the process. A process can have many threads, and they all share the same memory. Because they share everything, if one thread does something fatal, the entire process (and all its threads) crashes.

In one sentence: A process is the environment where things happen, and a thread is the thing actually doing the work inside that environment.