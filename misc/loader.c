#include <stdio.h>
#include <stdlib.h>
#include "../cpu/cpu_memory.h"
#include "../ppu/ppu_memory.h"

#define PRG_ROM_SIZE	0x4000
#define VROM_SIZE	0x2000
#define NES_HEADER_SIZE	0x10

static uint8_t mirroring;
static uint8_t fourScreen;

int load_rom(char *filename) {
	// Currently NROM support only
	FILE *fp;
	uint32_t addr;
	int i;
	uint8_t buf;

	uint8_t header[0x10];
	uint8_t numPRG; //numCHR;
	//uint8_t mirroring, fourScreen;
	//uint8_t batterySram;
	uint8_t trainer;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Failed to open input file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// Read header
	for (i = 0; i < NES_HEADER_SIZE; i++)
		fscanf(fp, "%c", &(header[i]));

	numPRG = header[4];
	//numCHR = header[5];
	mirroring = header[6] & 0x01;
	fourScreen = header[6] & 0x08;
	//batterySram = header[6] & 0x02;
	trainer = header[6] & 0x04;

	// Read trainer if present
	if (trainer)
		for (i = 0; i < 512; i++)
			fscanf(fp, "%*c");

	// Read first PRG-ROM bank into cpu memory
	for (addr = 0x8000; addr < 0xc000; addr++) {
		fscanf(fp, "%c", &buf);
		mem_write_cpu(addr, buf);
	}
	// Read second PRG-ROM bank into cpu memory
	if (numPRG == 2) {
		for (addr = 0xc000; addr < 0xffff; addr++) {
			fscanf(fp, "%c", &buf);
			mem_write_cpu(addr, buf);
		}
	} else {
		for (addr = 0xc000; addr < 0xffff; addr++) {
			mem_write_cpu(addr, mem_read_cpu(addr-0x4000));
		}
	}
	// Read VROM bank into PPU memory
	for (addr = 0; addr < VROM_SIZE; addr++) {
		fscanf(fp, "%c", &buf);
		mem_write_ppu(addr, buf);
	}

	return 0;
}

uint8_t get_mirroring() {
	return mirroring;
}

uint8_t get_four_screen() {
	return fourScreen;
}
