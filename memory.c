#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

#define PRG_ROM_SIZE	0x4000
#define VROM_SIZE	0x2000
#define NES_HEADER_SIZE	0x10
#define MAX_ROM_SIZE	0x100000

static uint8_t memory[MEM_SIZE];
static uint8_t memoryPPU[MEM_SIZE];
static uint8_t romImage[MAX_ROM_SIZE];

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

int mem_init() {
	//TODO
	uint8_t header[NES_HEADER_SIZE];
	int i;

	for (i = 0; i < MEM_SIZE; i++) {
		memory[i] = 0;
		memoryPPU[i] = 0;
	}

	// Read header
	memcpy(header, romImage, NES_HEADER_SIZE);
	// Read first PRG-ROM bank into memory
	memcpy(memory+0x8000, romImage + NES_HEADER_SIZE, PRG_ROM_SIZE);
	// Read second PRG-ROM bank into memory
	memcpy(memory+0xc000, romImage + NES_HEADER_SIZE + PRG_ROM_SIZE,
		PRG_ROM_SIZE);
	// Read VROM bank into PPU memory
	memcpy(memoryPPU, romImage + NES_HEADER_SIZE + PRG_ROM_SIZE*2, VROM_SIZE);

	return 0;
}

int load_rom(char *filename) {
	FILE *fp;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Failed to open input file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	fread(romImage, 1, MAX_ROM_SIZE, fp);

	fclose(fp);
}
