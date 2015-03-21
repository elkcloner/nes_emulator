#include "ppu_memory.h"
#include "../misc/loader.h"

#define SPRITE_MEM_SIZE	0xff

static uint8_t memoryPPU[MEM_SIZE_PPU];
static uint8_t memorySprite[SPRITE_MEM_SIZE];

uint8_t mem_read_ppu(uint16_t addr) {
	addr = addr % 0x4000;

	return memoryPPU[addr];
}

int mem_write_ppu(uint16_t addr, uint8_t value) {
	//TODO pattern table mirroring and palette mirroring
	addr = addr % 0x4000;

	if (addr < 0x2000) {
		// Pattern table
		memoryPPU[addr] = value;
	} else if (addr < 0x3f00) {
/*		// Name table
		if ((0x3000 <= addr) && (addr < 0x3f00))
			addr -= 0x1000;
		if ((0x2000 <= addr) && (addr < 0x2f00)) {
			memoryPPU[addr] = value;
			memoryPPU[addr + 0x1000] = value;
		} else {
			memoryPPU[addr] = value;
		}*/
		if ((0x3000 <= addr) && (addr < 0x3f00))
			addr -= 0x1000;

		if ((0x2000 <= addr) && (addr < 0x2400)) {
			// L1
			if (get_mirroring()) {
				memoryPPU[addr] = value;
				memoryPPU[addr+0x400] = value;
			} else {
				memoryPPU[addr] = value;
				memoryPPU[addr+0x800] = value;
			}
		} else if ((0x2400 <= addr) && (addr < 0x2800)) {
			// L2
			if (get_mirroring()) {
				memoryPPU[addr] = value;
				memoryPPU[addr-0x400] = value;
			} else {
				memoryPPU[addr] = value;
				memoryPPU[addr+0x800] = value;
			}
		} else if ((0x2800 <= addr) && (addr < 0x2c00)) {
			// L3
			if (get_mirroring()) {
				memoryPPU[addr] = value;
				memoryPPU[addr+0x400] = value;
			} else {
				memoryPPU[addr] = value;
				memoryPPU[addr-0x800] = value;
			}
		} else {
			// L4
			if (get_mirroring()) {
				memoryPPU[addr] = value;
				memoryPPU[addr-0x400] = value;
			} else {
				memoryPPU[addr] = value;
				memoryPPU[addr-0x800] = value;
			}
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

uint8_t read_sprite_mem(uint8_t addr) {
	return memorySprite[addr];
}

int write_sprite_mem(uint8_t addr, uint8_t value) {
	memorySprite[addr] = value;
	return 0;	
}
