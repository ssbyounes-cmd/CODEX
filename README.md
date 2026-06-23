1. We implemented the swapping of dongles for "even" coders so we can avoid the possibility of deadlock.

2. At the first compilation, we tend to leave odd coders compile first cuz order mangel that leaves dongles to starve (Avoid starvation)





The reason we added that fraction wait for even coders to resolve an issue of leaving coders to starve... 

WHen we throw all coders at once at the very start, sometimes even if I have enough dongles for 2 coders to compile,, only one compiles at 0ms and not the second even tho he should..

DId u forget the issue or should I show it to you again?

and please tell me if this issue is gonna resolve itself cuz of the scheduler implementation or is it deeper than that and should absolutely block even coders to not rush at first and wait for a fraction of a second