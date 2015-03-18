#include "ppu_memory.h"

static uint8_t memoryPPU[MEM_SIZE_PPU];

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
