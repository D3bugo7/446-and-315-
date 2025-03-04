# Makefile for peer.c

# Executable name
EXE = peer

# Compiler flags
CFLAGS = -Wall

# Linker flags
LDLIBS =

# Compiler
CC = gcc

.PHONY: all
all: $(EXE)

$(EXE): peer.o
    $(CC) $(CFLAGS) -o $(EXE) peer.o $(LDLIBS)

# Implicit rules defined by Make, but you can redefine if needed
peer.o: peer.c
    $(CC) $(CFLAGS) -c peer.c

.PHONY: clean
clean:
    rm -f $(EXE) *.o
