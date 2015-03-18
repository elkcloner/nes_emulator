#include "cpu_memory.h"

#define PRG_ROM_SIZE	0x4000
#define VROM_SIZE	0x2000
#define NES_HEADER_SIZE	0x10
#define MAX_ROM_SIZE	0x100000

static uint8_t memoryCPU[MEM_SIZE_CPU];

uint8_t mem_read_cpu(uint16_t addr) {
	return memoryCPU[addr];
}

int mem_write_cpu(uint16_t addr, uint8_t value) {
	uint32_t offset;

	if (addr < 0x2000) {
		// Mirror if writing to RAM
		addr = addr % 0x800;	
		for (offset = 0x0000; offset < 0x2000; offset += 0x0800)
			memoryCPU[addr + offset] = value;
	} else if ((0x2000 <= addr) && (addr < 0x4000)) {
		// Mirror if writing to I/O Registers 0x2000-0x4000
		addr = addr % 0x08;	
		for (offset = 0x2000; offset < 0x4000; offset += 0x08)
			memoryCPU[addr + offset] = value;
	} else {
		memoryCPU[addr] = value;
	}
	
	return 0;
}
