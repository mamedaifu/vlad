Schedule
========
1. Milestones 0 and 1 by the end Week 3.
2. Milestone 2 by early in Week 4.
3. Milestone 3 by the end of Week 4.
4. Milestone 4 by the deadline (Start of Week 5; Mon 31 August).

Pointer Arithmetic
==================
If you have a pointer ptr of type (T *), then ptr = ptr + 1 will add sizeof(T) bytes to ptr.
For the assignment, make sure that, if you want a pointer to a certain location in the memory block, you cast it to a byte pointer type, add the number of bytes (in our example 256), and then cast the resulting address to a pointer to a header struct, so you can initialise the header with the appropriate values.

Variations
==========
You will have noticed several points in the above discussion where behaviour of the program is not precisely specified, e.g.
- when to stop a search for a free region of the appropriate size
- which of the two free regions from a split region to choose for allocation
- where to leave the free_list_ptr positioned after vlad_malloc()
- where to leave the free_list_ptr positioned after vlad_free() 
The choices you make for these will result in different behaviours. Some choices will lead to less fragmentation for a certain mix of operations. Other choices may affect the subsequent free-list search time. Assuming they produce the required results, all of these choices are valid. You should, however, think about how the varations will affect the performance of your code, and try to choose one that gives good performance on all of the test you find/develop. 
