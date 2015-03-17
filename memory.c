#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

#define PRG_ROM_SIZE	0x4000
#define VROM_SIZE	0x2000
#define NES_HEADER_SIZE	0x10
#define MAX_ROM_SIZE	0x100000

static uint8_t memory[MEM_SIZE];
static uint8_t memoryPPU[MEM_SIZE_PPU];
static uint8_t romImage[MAX_ROM_SIZE];

uint8_t mem_read(uint16_t addr) {
	return memory[addr];
}

int mem_write(uint16_t addr, uint8_t value) {
	uint32_t offset;

	if (addr < 0x2000) {
		// Mirror if writing to RAM
		addr = addr % 0x800;	
		for (offset = 0x0000; offset < 0x2000; offset += 0x0800)
			memory[addr + offset] = value;
	} else if ((0x2000 <= addr) && (addr < 0x4000)) {
		// Mirror if writing to I/O Registers 0x2000-0x4000
		addr = addr % 0x08;	
		for (offset = 0x2000; offset < 0x4000; offset += 0x08)
			memory[addr + offset] = value;
	} else {
		memory[addr] = value;
	}
	
	return 0;
}

uint8_t mem_read_ppu(uint16_t addr) {
	addr = addr % 0x4000;

	return memoryPPU[addr];
}

int mem_write_ppu(uint16_t addr, uint8_t value) {
	//TODO
	addr = addr % 0x4000;

	if (addr < 0x2000) {
		// Pattern table
		memoryPPU[addr] = value;
	} else if (addr < 0x3f00) {
		// Name table
		if ((0x3000 <= addr) && (addr < 0x3f00))
			addr -= 0x1000;
		if ((0x2000 <= addr) && (addr < 0x2f00)) {
			memoryPPU[addr] = value;
			memoryPPU[addr + 0x1000] = value;
		} else {
			memoryPPU[addr] = value;
		}
	} else {
		// Palettes
		uint16_t offset;
		addr = addr % 0x20;
		for (offset = 0x3f00; offset < 0x4000; offset += 0x20)
			memoryPPU[addr + offset] = value;
	}
	
	return 0;
}

int mem_init() {
	//TODO
	uint8_t header[NES_HEADER_SIZE];
	int i;

	for (i = 0; i < MEM_SIZE; i++) {
		memory[i] = 0;
	}

	for (i = 0; i < MEM_SIZE_PPU; i++) {
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
