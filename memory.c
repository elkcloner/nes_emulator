#include <stdlib.h>
#include <stdio.h>
#include "memory.h"

#define PRG_ROM_SIZE	0x4000
#define VROM_SIZE	0x2000
#define NES_HEADER_SIZE	0x10

static uint8_t memory[MEM_SIZE];
static uint8_t memoryPPU[MEM_SIZE];

uint8_t mem_read(uint16_t addr) {
	//TODO
	return memory[addr];
}

int mem_write(uint16_t addr, uint8_t value) {
	//TODO
	memory[addr] = value;
	
	return 0;
}

uint8_t mem_read_ppu(uint16_t addr) {
	//TODO
	return memoryPPU[addr];
}

int mem_write_ppu(uint16_t addr, uint8_t value) {
	//TODO
	memoryPPU[addr] = value;
	
	return 0;
}

int mem_init(char *filename) {
	//TODO
	FILE *fp;
	uint8_t header[NES_HEADER_SIZE];
	int i;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Failed to open input file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < MEM_SIZE; i++) {
		memory[i] = 0;
		memoryPPU[i] = 0;
	}

	// Read header
	fread(header, 1, NES_HEADER_SIZE, fp);
	// Read first PRG-ROM bank into memory
	fread(memory + 0x8000, 1, PRG_ROM_SIZE, fp);
	// Read second PRG-ROM bank into memory
	fread(memory + 0xc000, 1, PRG_ROM_SIZE, fp);
	// Read VROM bank into PPU memory
	fread(memoryPPU, 1, VROM_SIZE, fp);

	return 0;
}
