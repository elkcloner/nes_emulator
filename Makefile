EXEC=emulator
CC=gcc
CFLAGS=-Wall -g
LIBS=-lSDL
HEADERS=cpu/cpu.h memory/memory.h cpu/instructions.h ppu/ppu.h misc/debug.h misc/display.h
OBJECTS=main.o cpu/cpu.o memory/memory.o cpu/instructions.o ppu/ppu.o misc/debug.o misc/display.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o ./*/*.o *~ $(EXEC)

remake: clean all
