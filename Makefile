EXEC=emulator
CC=gcc
CFLAG=-Wall -Wextra -pedantic
HEADERS=cpu.h instructions.h memory.h
OBJECTS=cpu.o instructions.o memory.o debug.o main.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@
	rm *.o

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *~ $(EXEC)
