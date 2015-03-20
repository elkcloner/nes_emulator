EXEC=emulator
CC=gcc
CFLAGS=-Wall -g
LIBS=-lSDL
HEADERS=cpu/cpu.h cpu/cpu_memory.h cpu/instructions.h ppu/ppu.h ppu/ppu_memory.h misc/debug.h misc/leader.h misc/display.h
OBJECTS=main.o cpu/cpu.o cpu/cpu_memory.o cpu/instructions.o ppu/ppu.o ppu/ppu_memory.o misc/debug.o misc/loader.o misc/display.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o ./*/*.o *~ $(EXEC)

remake: clean all
