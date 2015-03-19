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

/*	for (cycleCount = 0x8000; cycleCount < 0x10000; cycleCount++)
		printf("%02x ", mem_read_cpu(cycleCount));
*/

	cpu_init();

	cpu_interrupt(RESET);

	while (1) {
		cycleCount = 0;

		while ((cycleCount < CYCLES_PER_FRAME) && (get_ppu_changed() == NONE)) {
			print_debug();
			cycleCount +=  cpu_exec();
		}

		printf("ppu entered\n");
		ppu_run(cycleCount);
	}

	exit(EXIT_SUCCESS);
}
