CC=g++
CFLAGs=-03
EXTRAFLAGS=-pthread

.Phony: all clean

all: concurrency

concurrency: testConcurrency.cpp
	$(CC) $(CFLAGS) -o concurrency testConcurrency.cpp $(EXTRAFLAGS)

clean:
	rm -f concurrency *~
