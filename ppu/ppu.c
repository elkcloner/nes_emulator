#include <stdint.h>
#include "ppu.h"
#include "ppu_memory.h"
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

static uint8_t ppuScrollX = 0x00;
static uint8_t ppuScrollY = 0x00;
static uint16_t ppuAddrLatch = 0x0000;
static uint8_t scrollLatchStatus = 0;
static uint8_t addrLatchStatus = 0;

int ppu_run(int cycles) {
	int counter = cycles*3;

	while (counter > 0) {
		if (scanCycle > 340) {
			scanCycle = 0;
			scanLine++;
			if (scanLine > 261)
				scanLine = 0;
		}	

		// 1,241 Set vBlank and send interrup
		if ((scanLine == 241) && (scanCycle == 1)) {
			cpu_interrupt(NMI);
			set_vBlank();
		}

		// 1,261 clear PPUSTATUS bit 7
		registers.status &= ~0x80;

		scanCycle++;
		counter--;
	}

	// TODO deal with status changes
	switch (get_ppu_changed()) {
		case NONE:
			break;
		case WRITE_2000:
			// update PPUCTRL value
			registers.controller = mem_read_cpu(0x2000);	
			break;
		case WRITE_2001:
			// update PPUMASK value
			registers.mask = mem_read_cpu(0x2001);
			break;
		case READ_2002:
			// clear PPUSTATUS bit 7
			registers.status &= ~0x80;
			// clear address latch for scroll and addr
			ppuScrollX = 0x00;
			ppuScrollY = 0x00;
			ppuAddrLatch = 0x00;
			scrollLatchStatus = 0;
			addrLatchStatus = 0;
			break;
		case WRITE_2003:
			// update OAMADDR
			// update OAMDATA
			registers.oamAddress = mem_read_cpu(0x2003);
			registers.oamData = read_sprite_mem(registers.oamAddress);
			break;
		case WRITE_2004:
			// write OAMdata to addr OAMaddr, increment OAMAddr
			write_sprite_mem(registers.oamAddress, mem_read_cpu(0x2004));
			break;
		case READ_2004:
			// nothing	
			break;
		case WRITE_2005:
			// write twice for PPUSCROLL addr latch
			if (scrollLatchStatus == 1)
				ppuScrollX = mem_read_cpu(0x2005);
			else
				ppuScrollY = mem_read_cpu(0x2005);
			scrollLatchStatus++;
			break;
		case WRITE_2006:
			// write twice for PPUADDR latch
			if (addrLatchStatus == 1)
				ppuAddrLatch = mem_read_cpu(0x2005) << 8;
			else
				ppuAddrLatch |= mem_read_cpu(0x2005);
			addrLatchStatus++;
			break;
		case WRITE_2007:
			// write to VRAM at addr latch
			mem_write_ppu(ppuAddrLatch, mem_read_cpu(0x2007));
			break;
		case READ_2007:
			// update to contain data at ppuaddr
			registers.data = mem_read_ppu(ppuAddrLatch);	
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
	mem_write_cpu(0x2004, read_sprite_mem(registers.oamAddress));	
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
