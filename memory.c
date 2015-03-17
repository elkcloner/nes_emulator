#include <stdlib.h>
#include <stdio.h>
#include "memory.h"

static uint8_t memory[MEM_SIZE];

uint8_t mem_read(uint16_t addr) {
	//TODO
	return memory[addr];
}

int mem_write(uint16_t addr, uint8_t value) {
	//TODO
	memory[addr] = value;
	
	return 0;
}

int mem_init(char *filename) {
	//TODO
	FILE *fp;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Failed to open input file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	int i, n;

	for (i = 0; i < MEM_SIZE; i++)
		memory[i] = 0;

	n = fread(memory, 1, MEM_SIZE, fp);
	memory[n-1] = 0;

	return 0;
}
