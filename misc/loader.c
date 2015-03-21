#include <stdio.h>
#include <stdlib.h>
#include "../cpu/cpu_memory.h"
#include "../ppu/ppu_memory.h"

#define PRG_ROM_SIZE	0x4000
#define VROM_SIZE	0x2000
#define NES_HEADER_SIZE	0x10

int load_rom(char *filename) {
	FILE *fp;
	uint32_t addr;
	int i;
	uint8_t buf;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Failed to open input file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// Read header
	for (i = 0; i < NES_HEADER_SIZE; i++)
		fscanf(fp, "%*c");
	// Read first PRG-ROM bank into cpu memory
/*	for (addr = 0x8000; addr < 0x8000 + PRG_ROM_SIZE; addr++) {
		fscanf(fp, "%c", &buf);
		mem_write_cpu(addr, buf);
	}*/
	// Read second PRG-ROM bank into cpu memory
	for (addr = 0xc000; addr < 0xc000 + PRG_ROM_SIZE; addr++) {
		fscanf(fp, "%c", &buf);
		mem_write_cpu(addr, buf);
	}
	// Read VROM bank into PPU memory
	for (addr = 0; addr < VROM_SIZE; addr++) {
		fscanf(fp, "%c", &buf);
		mem_write_ppu(addr, buf);
	}

	return 0;
}
