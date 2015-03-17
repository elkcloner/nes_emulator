#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "memory.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	load_rom(argv[1]);
	cpu_reset();

	int i;
	/*for (i = 0; i < 0x100; i++)
		printf("%02x ", mem_read(0x8000+i));
*/

	while (1) {
		print_registers();
		//print_memory();
		getc(stdin);
	
		cpu_exec();
	}
}
