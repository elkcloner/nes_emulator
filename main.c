#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "memory.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	cpu_init();
	opcodes_init();
	mem_init(argv[1]);

	/*while (1) {
		print_registers();
		print_memory();
		getc(stdin);
	
		cpu_exec();
	}*/

	opcodes_clean();	
	exit(EXIT_SUCCESS);
}
