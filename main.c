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
	cpu_reset();

	// values for testing
	// remember to revert loader
	set_pc(0xc000);
	set_sp(0xfd);
	set_status(0x24);

	int i;
	for (i = 0; i < 8991; i++) {
		print_debug();
		//print_memory();
		//getc(stdin);
	
		cpu_exec();
	}


	exit(EXIT_SUCCESS);
}
