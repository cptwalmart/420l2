CFLAGS=-std=c99
LIBS=-lm
CC=mpicc
all: test test2

test: parta.o mfunctions.o
	$(CC) -o $@ $(CFLAGS) $^ $(LIBS)

test2: main2.o mfunctions.o
	$(CC) -o $@ $(CFLAGS) $^ $(LIBS)