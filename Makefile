CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -pthread

all: server

server: server.cpp table.h account.h account.cpp position.h position.cpp transaction.h transaction.cpp functions.h functions.cpp execution.h execution.cpp
	$(CC) $(CFLAGS) -o server server.cpp account.cpp position.cpp transaction.cpp functions.cpp execution.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o server

clobber:
	rm -f *~ *.o
