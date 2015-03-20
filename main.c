#include <stdlib.h>
#include <stdio.h>
#include "cpu/cpu.h"
#include "cpu/cpu_memory.h"
#include "ppu/ppu.h"
#include "ppu/ppu_memory.h"
#include "misc/loader.h"
#include "misc/debug.h"
#include "misc/display.h"

#define CYCLES_PER_FRAME	29871

int main(int argc, char **argv) {
	int cycleCount;	

	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	display_init();

	load_rom(argv[1]);

/*	for (cycleCount = 0xc000; cycleCount < 0x10000; cycleCount++)
		printf("%02x ", mem_read_cpu(cycleCount));
*/

	cpu_init();

	cpu_interrupt(RESET);

	while (1) {
		cycleCount = 0;

		while ((cycleCount < CYCLES_PER_FRAME) && (get_ppu_changed() == NONE)) {
			cycleCount +=  cpu_exec();
		}

		//printf("ppu entered\n");
		ppu_run(cycleCount);

		if (display_events())
			break;
	}

	int i,j;
/*	printf("Pattern Table 1:\n");
	for (i = 0; i < 0x1000; i+=16) {
		printf("0x%02x\n\tLow :", i/16);
		for (j = 0; j < 8; j++)
			printf(" %02x", mem_read_ppu(0x1000 + i + j));
		printf("\n\tHigh:");
		for (j = 0; j < 8; j++)
			printf(" %02x", mem_read_ppu(0x1000 + i + j + 8));
		printf("\n");
	}
	printf("\n");
	printf("Name Table 0:\n");
	for (i = 0; i < 30; i++) {
		for (j = 0; j < 32; j++) {
			printf("%02x ", mem_read_ppu(0x2000+i*32+j));
		}
		printf("\n");
	}*/
	printf("\n");
	printf("Attribute Table:\n");
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			printf("%02x ", mem_read_ppu(i*8 + j + 0x23c0));
		printf("\n");
	}
	printf("\n");
	printf("Palette Table:\n");
	for (i = 0; i < 0x10; i++)
		printf("%02x ",mem_read_ppu(0x3f00 + i));
	printf("\n");

	exit(EXIT_SUCCESS);
}
