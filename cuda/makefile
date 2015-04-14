# Makefile
#
# for cuda implementation of image filter

# Compilers
CC=gcc
NVCC=nvcc

# Compiler Flags
CFLAGS= -std=gnu99 -Wall -Wextra -Wmissing-prototypes

#Linked Libraries
LDLIBS= -L/opt/cuda-toolkit/lib64 -lcuda -lcudart

# Variables
APPLICATION=imagfil

OBJECTS=main.o image.o filter.o filter_kernel.o

#Application name
$(APPLICATION): $(OBJECTS)
	$(NVCC) -o $@ $(OBJECTS)
	
main.o: src/main.c src/image.h src/filter.h 
	$(CC) $(CFLAGS) -c src/main.c
	
image.o: src/image.h src/filter.h
	$(CC) $(CFLAGS) -c src/image.c

filter.o: src/filter.c src/filter.h
	$(CC) $(CFLAGS) -c src/filter.c 

filter_kernel.o: src/filter_kernel.cu src/filter.h
	$(NVCC) -c src/filter_kernel.cu

PHONY: clean install

clean:
	- rm *.o
	- rm -rf bin
	- rm $(APPLICATION)
	
install:
	mkdir bin
	- mv *.o bin
	- mv src/*.o bin
	
build:
	make clean #ignore if bin file or executables not exist
	make
	make install

