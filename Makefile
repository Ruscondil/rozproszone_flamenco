SOURCES=$(wildcard *.c)
HEADERS=$(SOURCES:.c=.h)
#FLAGS=-DDEBUG -g
FLAGS=-g

all: main

main: $(SOURCES) $(HEADERS) Makefile
	mpicc $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

run: main Makefile tags
	mpirun -np 7 ./main 3 2 2 2
