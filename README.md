# IntrusivePriorityQueue

A C++ implementation of an intrusive priority queue that also enables enqued items to be erased or reprioritized.

The objective is to be as fast as possible in an environment with a high rate of enqueuing, dequeuing, and priority changes. Using an array of random numbers as input, the Intrusive Priority Queue’s performance is two times faster than the C++ std::priority_queue for push operations and equivalent for pop operations.  The additional reprioritize and erase operations are also O(log(n)) and 1.5 times the insert rate.

Included is the test program that was used to compare performance of the intrusive priority queue relative to the std::priority_queue, std::set, and an intrusive linked list.
