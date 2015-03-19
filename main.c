#include <stdlib.h>
#include <stdio.h>
#include "cpu/cpu.h"
#include "cpu/cpu_memory.h"
#include "ppu/ppu.h"
#include "ppu/ppu_memory.h"
#include "misc/loader.h"
#include "misc/debug.h"

#define CYCLES_PER_FRAME	29871

int main(int argc, char **argv) {
	int cycleCount;	

	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	load_rom(argv[1]);
	cpu_init();

	cpu_interrupt(RESET);

	while (1) {
		cycleCount =  cpu_run(CYCLES_PER_FRAME);
		ppu_run(cycleCount);
	}


	exit(EXIT_SUCCESS);
}
