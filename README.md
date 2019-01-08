Image Filter
====================

Written for the Pareller Programming course of my MSc. Implemented a simple image filter in three different parallel
programming models and one sequential form.

Image Filtering
---------------
The image filtering is accomplished by means of convolution between a kernel and
an image. The values of a given pixel in the output image are calculated by
multiplying each kernel value by the corresponding input image pixel values.
For the calculation of the edge pixels the extend method is used.

OPENMP
------
OpenMP is an API that supports multi-platform shared memory multiprocessing
programming in C, C++. It is an implementation of multithreading, a method of
parallelizing whereby a master thread (a series of instructions executed
consecutively) forks a specified number of slave threads and the system divides
a task among them. The threads then run concurrently, with the runtime
environment allocating threads to different processors.

For the thread creation, synchronization e.t.c the pre-processor directive
\#pragma is used

MPI
---
Message Passing Interface (MPI) is a standardized and portable message-passing
standard. It is a language-independent communications protocol used to program
parallel computers. Both point-to-point and collective communication are
supported. MPI "is a message-passing application programmer interface, together
with protocol and semantic specifications for how its features must behave in
any implementation.

CUDA
----
CUDA (after the Plymouth Barracuda), which stands for Compute Unified Device
Architecture, is a parallel computing platform and programming model created by
NVIDIA and implemented by the graphics processing units (GPUs) that they
produce. CUDA gives developers direct access to the virtual instruction set and
memory of the parallel computational elements in CUDA GPUs.