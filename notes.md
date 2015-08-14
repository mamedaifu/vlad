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
