EXE = h1-counter
CFLAGS = -Wall
LDLIBS =
CC = gcc

.PHONY: all
all: $(EXE)

$(EXE): h1-counter.o
    $(CC) $(CFLAGS) -o $(EXE) h1-counter.o $(LDLIBS)
# Implicit rules defined by Make, but you can redefine if needed
#
h1-counter.o: h1-counter.c
    $(CC) $(CFLAGS) -c h1-counter.c

.PHONY: clean
clean:
    rm -f $(EXE) *.o
