CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(imagefil)

#set sources dir
set(SRC src)

set(SPEXEC single_process)
add_executable(${SPEXEC} 
${SRC}/single_process/main.c
${SRC}/single_process/image.c
${SRC}/single_process/filter.c
) 

set(OMPEXEC omp)
add_executable(${OMPEXEC} 
${SRC}/openmp/main.c
${SRC}/openmp/image.c
${SRC}/openmp/filter.c
)

set(MPIEXEC mpi)
add_executable(${MPIEXEC} 
${SRC}/mpi/main.c
${SRC}/mpi/image.c
${SRC}/mpi/filter.c
)

#[[set(CUDAEXEC cuda)
add_executable(${CUDAEXEC} 
${SRC}/cuda/main.c
${SRC}/cuda/image.c
${SRC}/cuda/filter.c
)]]