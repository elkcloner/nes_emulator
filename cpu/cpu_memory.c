#include "cpu_memory.h"

#define PRG_ROM_SIZE	0x4000
#define VROM_SIZE	0x2000
#define NES_HEADER_SIZE	0x10
#define MAX_ROM_SIZE	0x100000

static uint8_t memoryCPU[MEM_SIZE_CPU];
static ppuChange change = NONE;

uint8_t mem_read_cpu(uint16_t addr) {
	if (addr == 0x2002)
		set_ppu_changed(READ_2002);
	else if (addr == 0x2004)
		set_ppu_changed(READ_2004);
	else if (addr == 0x2007)
		set_ppu_changed(READ_2007);

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

		switch(addr) {
			case 0x00:
				set_ppu_changed(WRITE_2000);
				break;
			case 0x01:
				set_ppu_changed(WRITE_2001);
				break;
			case 0x03:
				set_ppu_changed(WRITE_2003);
				break;
			case 0x04:
				set_ppu_changed(WRITE_2004);
				break;
			case 0x05:
				set_ppu_changed(WRITE_2005);
				break;
			case 0x06:
				set_ppu_changed(WRITE_2006);
				break;
			case 0x07:
				set_ppu_changed(WRITE_2007);
				break;
		}

		for (offset = 0x2000; offset < 0x4000; offset += 0x08)
			memoryCPU[addr + offset] = value;
	} else {
		if (addr == 0x4014)
			set_ppu_changed(WRITE_4014);

		memoryCPU[addr] = value;
	}
	
	return 0;
}

ppuChange get_ppu_changed() {
	return change;
}

int set_ppu_changed(ppuChange newChange) {
	change = newChange;
	return 0;
}
