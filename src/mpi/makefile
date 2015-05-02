# Makefile
#
# for single process/thread implementation of image filter

# Compiler
CC=mpicc

#Compiler Flags
CFLAGS= -std=gnu99 -Wall -Wextra -Wmissing-prototypes

#Linked Libraries
LDLIBS= -lm

# Variables
APPLICATION=imagfil

OBJECTS=main.o src/image.o src/filter.o

#Application name
$(APPLICATION): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) -o $@
	
main.o: src/main.c src/image.h src/filter.h 
	$(CC) $(CFLAGS) -c src/main.c
	
image.o: src/image.h src/filter.h
	$(CC) $(CFLAGS) -c src/image.c

filter.o: src/filter.c src/filter.h
	$(CC) $(CFLAGS) -c src/filter.c 

PHONY: clean install

clean:
	rm -rf bin
	rm $(APPLICATION)
	
install:
	mkdir bin
	mv *.o bin
	mv src/*.o bin
	
build:
	- make clean #ignore if bin file or executables not exist
	make
	make install

