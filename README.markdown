#Introduction

When one deals with asynchronous I/O, __send and acknowledge__ mechanism is frequently used.
Due to memory limitation, a timeouted sent data has to be aborted. Such operation can 
be support by random accesible container. Though, the operation potentialy requires longer 
time and resource to be accomplished. 

Instead, mbuf uses multiple `std::deque` containers to store sent data that wait for 
acknowledgement. In normal case that acknowledgements are in the same order to sent 
sequence, the deque can offer optimal performance for accessing end points. To reduce the 
overhead of abort operation, mbuf store sent data circularily among the containers (see 
below) such that increase probability of acknowledged data be in front of a deque.

    -------------------
    | 0 | 2 | 4 | ... |
    -------------------

    -------------------
    | 1 | 3 | 5 | ... |
    -------------------


