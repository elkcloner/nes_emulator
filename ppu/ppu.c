#include <stdint.h>
#include "ppu.h"
#include "../cpu/cpu.h"
#include "../cpu/cpu_memory.h"

typedef struct {
	uint8_t controller;		// $2000
	uint8_t mask;			// $2001
	uint8_t status;			// $2002
	uint8_t oamAddress;		// $2003
	uint8_t oamData;		// $2004
	uint8_t scroll;			// $2005
	uint8_t address;		// $2006
	uint8_t data;			// $2007

} PPURegisters;

static PPURegisters registers;

static int scanLine = 0;
static int scanCycle = 0;

int ppu_run(int cycles) {
	int counter = cycles*3;

	while (counter > 0) {
		if (scanCycle > 340) {
			scanCycle = 0;
			scanLine++;
		}	
		if (scanLine > 261)
			scanLine = 0;

		if ((scanLine == 241) && (scanCycle == 1)) {
			cpu_interrupt(NMI);
			set_vBlank();
		}

		scanCycle++;
		counter--;
	}

	// TODO deal with status changes
	switch (get_ppu_changed()) {
		case NONE:
			break;
		case WRITE_2000:
			break;
		case WRITE_2001:
			break;
		case READ_2002:
			break;
		case WRITE_2003:
			break;
		case WRITE_2004:
			break;
		case READ_2004:
			break;
		case WRITE_2005:
			break;
		case WRITE_2006:
			break;
		case WRITE_2007:
			break;
		case READ_2007:
			break;
		case WRITE_4014:
			break;
	}

	update_memory();
	set_ppu_changed(NONE);

	return 0;
}

int update_memory() {
	mem_write_cpu(0x2000, registers.controller);	
	mem_write_cpu(0x2001, registers.mask);	
	mem_write_cpu(0x2002, registers.status);	
	mem_write_cpu(0x2003, registers.oamAddress);	
	mem_write_cpu(0x2004, registers.oamData);	
	mem_write_cpu(0x2005, registers.scroll);	
	mem_write_cpu(0x2006, registers.address);	
	mem_write_cpu(0x2007, registers.data);	

	return 0;
}

int set_vBlank() {
	registers.status |= 0x80;	
	return 0;
}

int clr_vBlank() {
	registers.status &= ~0x80;	
	return 0;
}
