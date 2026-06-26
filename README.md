1. We implemented the swapping of dongles for "even" coders so we can avoid the possibility of deadlock.

2. We stagger the thread start times (delaying even coders slightly at T=0ms) to guarantee maximum table throughput and prevent the 'roadblock effect'.

3. take_dongle() and release_dongle() are two protected operations that ensure that only one thread can take or release a dongle at a time, preventing race conditions

- cond_wait() is used to block one thread while another thread is using a dongle, ensuring that threads wait their turn to access shared resources.

- cond_signal() is used to wake up a waiting thread when a dongle becomes available.

