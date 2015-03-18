#include <stdlib.h>
#include <stdio.h>
#include "cpu/cpu.h"
#include "cpu/cpu_memory.h"
#include "ppu/ppu_memory.h"
#include "misc/loader.h"
#include "misc/debug.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	load_rom(argv[1]);
	cpu_init();

	cpu_interrupt(RESET);

	while (1) {
		print_debug();
		getc(stdin);
	
		cpu_exec();
	}


	exit(EXIT_SUCCESS);
}
