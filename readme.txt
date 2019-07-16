This is an implementation of a userspace buddy system memory allocator for C++, written in C++ for Linux.
It can be easily ported to Windows, but I have not done that at this point.

To build, do:
make

Then grab the newly-built static library (lib/libMyMalloc.a) and the public library header (mem.h), and include those into your own project.