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
// TODO this is an ugly fix
static uint8_t hasRead = 0;

int ppu_run(int cycles) {
	int counter = cycles*3;

	while (counter > 0) {
		if (scanCycle > 340) {
			scanCycle = 0;
			scanLine++;
			if (scanLine > 261) {
				scanLine = 0;
			}
		}	

		// 0,240 render the entire frame based off NT0
		if ((scanLine == 240) && (scanCycle == 1))
			render_frame();

		// 1,241 Set vBlank and send interrupt
		if ((scanLine == 241) && (scanCycle == 1)) {
			if (registers.controller & 0x80)
				cpu_interrupt(NMI);
			set_vBlank();
			hasRead = 0;
		}

		// 1,261 clear PPUSTATUS bit 7
		if ((scanLine == 261) && (scanCycle == 1))
			clr_vBlank();

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
			if (hasRead) {
				registers.status &= ~0x80;
			}
			hasRead = 1;
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

int render_frame() {
	ppuChange temp = get_ppu_changed();

	uint8_t frame[256][256];
	uint8_t patternLow, patternHigh;

	uint16_t ptPointer;
	uint8_t attributes;

	int rowCount = 0;
	int colCount = 0;
	int i, j;

	for (i = 0; i < 240; i++)
		for (j = 0; j < 256; j++)
			frame[i][j] = 0;

	// Populate frame with bits from the pattern table
	for (rowCount = 0; rowCount < 30; rowCount++) {
		for (colCount = 0; colCount < 32; colCount++) {
			// Get index into the pattern table
			ptPointer = mem_read_ppu(rowCount*32 + colCount);
			for (i = 0; i < 8; i++) {
				patternLow = mem_read_ppu(0x2000 + ptPointer*16 + i);
				patternHigh = mem_read_ppu(0x2000 + ptPointer*16 + 8 + i);

				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x80) > 7;
				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x40) > 6;
				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x20) > 5;
				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x10) > 4;
				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x08) > 3;
				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x04) > 2;
				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x02) > 1;
				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x01);
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x80) > 6;
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x40) > 5;
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x20) > 4;
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x10) > 3;
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x08) > 2;
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x04) > 1;
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x02);
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x01) < 1;
			}
		}
	}

	// Add bits from the attribute table to the frame
	for (rowCount = 0; rowCount < 8; rowCount++) {
		for (colCount = 0; colCount < 8; colCount++) {
			attributes = mem_read_ppu(0x23c0 + rowCount*8 + colCount);	
			for (i = 0; i < 32; i++) {
				for (j = 0; j < 32; j++) {
					if ((i < 16) && (j < 16)) {
						// upper left tiles	
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0x03) < 2;
					} else if ((i < 16) && (j >= 16)) {
						// upper right tiles
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0x0c);
					} else if ((i >= 16) && (j < 16)) {
						// lower left tiles
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0x30) > 2;
					} else {
						// lower right tiles
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0xc0) > 4;
					}
				}
			}
		}

	}

	// load frame with values from palettes
	for (rowCount = 0; rowCount < 240; rowCount++) {
		for (colCount = 0; colCount < 256; colCount++) {
			frame[rowCount][colCount] = mem_read_ppu(0x3f00 +
				((uint16_t) frame[rowCount][colCount]));
		}
	}

	set_ppu_changed(temp);
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
