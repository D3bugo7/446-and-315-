# Makefile
CC = gcc
CFLAGS = -Wall

peer: peer.o network.o protocol.o
    $(CC) $(CFLAGS) -o peer peer.o network.o protocol.o

peer.o: peer.c network.h protocol.h
    $(CC) $(CFLAGS) -c peer.c

network.o: network.c network.h
    $(CC) $(CFLAGS) -c network.c

protocol.o: protocol.c protocol.h
    $(CC) $(CFLAGS) -c protocol.c

clean:
    rm -f peer *.o
