CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq

all: test

test: main.cpp table.h account.h account.cpp position.h position.cpp transaction.h transaction.cpp functions.h functions.cpp
	$(CC) $(CFLAGS) -o test main.cpp account.cpp position.cpp transaction.cpp functions.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o test

clobber:
	rm -f *~ *.o
