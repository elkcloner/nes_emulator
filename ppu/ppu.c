#include <stdint.h>
#include <stdio.h>
#include "ppu.h"
#include "../cpu/cpu.h"
#include "../misc/display.h"
#include "../memory/memory.h"

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

static char pixels[256*240*3*9];

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

		// 0,240 render the entire frame
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
			registers.status = (registers.status & 0xf0) | (mem_read_cpu(0x2000) & 0x0f);
			break;
		case WRITE_2001:
			// update PPUMASK value
			registers.mask = mem_read_cpu(0x2001);
			registers.status = (registers.status & 0xf0) | (mem_read_cpu(0x2001) & 0x0f);
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
			registers.oamData = mem_read_oam(registers.oamAddress);
			registers.status = (registers.status & 0xf0) | (mem_read_cpu(0x2003) & 0x0f);
			break;
		case WRITE_2004:
			// write OAMdata to addr OAMaddr, increment OAMAddr
			mem_write_oam(registers.oamAddress, mem_read_cpu(0x2004));
			registers.status = (registers.status & 0xf0) | (mem_read_cpu(0x2004) & 0x0f);
			break;
		case READ_2004:
			// nothing	
			break;
		case WRITE_2005:
			// write twice for PPUSCROLL addr latch
			if (scrollLatchStatus == 0) {
				ppuScrollX = mem_read_cpu(0x2005);
				scrollLatchStatus = 1;
			} else {
				ppuScrollY = mem_read_cpu(0x2005);
				scrollLatchStatus = 0;
			}
			registers.status = (registers.status & 0xf0) | (mem_read_cpu(0x2005) & 0x0f);
			break;
		case WRITE_2006:
			// write twice for PPUADDR latch
			if (addrLatchStatus == 0) {
				ppuAddrLatch = ((uint16_t)mem_read_cpu(0x2006)) << 8;
				addrLatchStatus = 1;
			} else {
				ppuAddrLatch = ppuAddrLatch | ((uint16_t)mem_read_cpu(0x2006));
				addrLatchStatus = 0;
			}
			registers.status = (registers.status & 0xf0) | (mem_read_cpu(0x2006) & 0x0f);
			break;
		case WRITE_2007:
			// write to VRAM at addr latch
			//printf("0x%04x: %02x\t%d\n", ppuAddrLatch, mem_read_cpu(0x2007), addrLatchStatus);	
			mem_write_ppu(ppuAddrLatch, mem_read_cpu(0x2007));
			if (registers.controller & 0x4)
				ppuAddrLatch += 0x20;
			else
				ppuAddrLatch += 0x1;
			registers.status = (registers.status & 0xf0) | (mem_read_cpu(0x2007) & 0x0f);
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

	uint16_t nameTable, patternTable;

	uint16_t ptPointer;
	uint16_t attributes;

	int rowCount = 0;
	int colCount = 0;
	int i, j;

	switch (registers.controller & 0x3) {
		case 0:
			nameTable = 0x2000;
			break;
		case 1:
			nameTable = 0x2400;
			break;
		case 2:
			nameTable = 0x2800;
			break;
		case 3:
			nameTable = 0x2c00;
			break;
	}

	if (registers.controller & 0x10)
		patternTable = 0x1000;
	else
		patternTable = 0x0000;

	for (i = 0; i < 240; i++)
		for (j = 0; j < 256; j++)
			frame[i][j] = 0;

	// Populate frame with bits from the pattern table
	for (rowCount = 0; rowCount < 30; rowCount++) {
		for (colCount = 0; colCount < 32; colCount++) {
			// Get index into the pattern table
			ptPointer = mem_read_ppu(nameTable + rowCount*32 + colCount);
			for (i = 0; i < 8; i++) {
				patternLow = mem_read_ppu(patternTable + ptPointer*16 + i);
				patternHigh = mem_read_ppu(patternTable + ptPointer*16 + 8 + i);

				frame[rowCount*8+i][colCount*8+0] = (patternLow & 0x80) >> 7;
				frame[rowCount*8+i][colCount*8+1] = (patternLow & 0x40) >> 6;
				frame[rowCount*8+i][colCount*8+2] = (patternLow & 0x20) >> 5;
				frame[rowCount*8+i][colCount*8+3] = (patternLow & 0x10) >> 4;
				frame[rowCount*8+i][colCount*8+4] = (patternLow & 0x08) >> 3;
				frame[rowCount*8+i][colCount*8+5] = (patternLow & 0x04) >> 2;
				frame[rowCount*8+i][colCount*8+6] = (patternLow & 0x02) >> 1;
				frame[rowCount*8+i][colCount*8+7] = (patternLow & 0x01);
				frame[rowCount*8+i][colCount*8+0] |= (patternHigh & 0x80) >> 6;
				frame[rowCount*8+i][colCount*8+1] |= (patternHigh & 0x40) >> 5;
				frame[rowCount*8+i][colCount*8+2] |= (patternHigh & 0x20) >> 4;
				frame[rowCount*8+i][colCount*8+3] |= (patternHigh & 0x10) >> 3;
				frame[rowCount*8+i][colCount*8+4] |= (patternHigh & 0x08) >> 2;
				frame[rowCount*8+i][colCount*8+5] |= (patternHigh & 0x04) >> 1;
				frame[rowCount*8+i][colCount*8+6] |= (patternHigh & 0x02);
				frame[rowCount*8+i][colCount*8+7] |= (patternHigh & 0x01) << 1;
			}
		}
	}

	// Add bits from the attribute table to the frame
	for (rowCount = 0; rowCount < 8; rowCount++) {
		for (colCount = 0; colCount < 8; colCount++) {
			attributes = mem_read_ppu(nameTable + 0x3c0 + rowCount*8 + colCount);	
			for (i = 0; i < 32; i++) {
				for (j = 0; j < 32; j++) {
					if ((i < 16) && (j < 16)) {
						// upper left tiles	
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0x03) << 2;
					} else if ((i < 16) && (j >= 16)) {
						// upper right tiles
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0x0c);
					} else if ((i >= 16) && (j < 16)) {
						// lower left tiles
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0x30) >> 2;
					} else {
						// lower right tiles
						frame[rowCount*32+i][colCount*32+j] |= (attributes & 0xc0) >> 4;
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

	// convert frame data into pixels for sdl
/*	for (i = 0; i < 240; i++) {
		for (j = 0; j < 256; j++) {
			switch (frame[i][j]) {
				case 0x00:
					pixels[(i*256+j)*3+0] = 0x75;
					pixels[(i*256+j)*3+1] = 0x75;
					pixels[(i*256+j)*3+2] = 0x75;
					break;
				case 0x01:
					pixels[(i*256+j)*3+0] = 0x27;
					pixels[(i*256+j)*3+1] = 0x1b;
					pixels[(i*256+j)*3+2] = 0x8f;
					break;
				case 0x02:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0xab;
					break;
				case 0x03:
					pixels[(i*256+j)*3+0] = 0x47;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x9f;
					break;
				case 0x04:
					pixels[(i*256+j)*3+0] = 0x8f;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x77;
					break;
				case 0x05:
					pixels[(i*256+j)*3+0] = 0xab;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x13;
					break;
				case 0x06:
					pixels[(i*256+j)*3+0] = 0xa7;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x07:
					pixels[(i*256+j)*3+0] = 0x7f;
					pixels[(i*256+j)*3+1] = 0x0b;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x08:
					pixels[(i*256+j)*3+0] = 0x43;
					pixels[(i*256+j)*3+1] = 0x2f;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x09:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x47;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x0a:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x51;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x0b:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x3f;
					pixels[(i*256+j)*3+2] = 0x17;
					break;
				case 0x0c:
					pixels[(i*256+j)*3+0] = 0x1b;
					pixels[(i*256+j)*3+1] = 0x3f;
					pixels[(i*256+j)*3+2] = 0x5f;
					break;
				case 0x0d:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x0e:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x0f:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x10:
					pixels[(i*256+j)*3+0] = 0xbc;
					pixels[(i*256+j)*3+1] = 0xbc;
					pixels[(i*256+j)*3+2] = 0xbc;
					break;
				case 0x11:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x73;
					pixels[(i*256+j)*3+2] = 0xef;
					break;
				case 0x12:
					pixels[(i*256+j)*3+0] = 0x23;
					pixels[(i*256+j)*3+1] = 0x3b;
					pixels[(i*256+j)*3+2] = 0xef;
					break;
				case 0x13:
					pixels[(i*256+j)*3+0] = 0x83;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0xf3;
					break;
				case 0x14:
					pixels[(i*256+j)*3+0] = 0xbf;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0xbf;
					break;
				case 0x15:
					pixels[(i*256+j)*3+0] = 0xe7;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x5b;
					break;
				case 0x16:
					pixels[(i*256+j)*3+0] = 0xdb;
					pixels[(i*256+j)*3+1] = 0x2b;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x17:
					pixels[(i*256+j)*3+0] = 0xcb;
					pixels[(i*256+j)*3+1] = 0x4f;
					pixels[(i*256+j)*3+2] = 0x0f;
					break;
				case 0x18:
					pixels[(i*256+j)*3+0] = 0x8b;
					pixels[(i*256+j)*3+1] = 0x73;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x19:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x97;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x1a:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0xab;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x1b:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x93;
					pixels[(i*256+j)*3+2] = 0x3b;
					break;
				case 0x1c:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x83;
					pixels[(i*256+j)*3+2] = 0x8b;
					break;
				case 0x1d:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x1e:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x1f:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x20:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0xff;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x21:
					pixels[(i*256+j)*3+0] = 0x3f;
					pixels[(i*256+j)*3+1] = 0xbf;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x22:
					pixels[(i*256+j)*3+0] = 0x5f;
					pixels[(i*256+j)*3+1] = 0x97;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x23:
					pixels[(i*256+j)*3+0] = 0xa7;
					pixels[(i*256+j)*3+1] = 0x8b;
					pixels[(i*256+j)*3+2] = 0xfd;
					break;
				case 0x24:
					pixels[(i*256+j)*3+0] = 0xf7;
					pixels[(i*256+j)*3+1] = 0x7b;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x25:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0x77;
					pixels[(i*256+j)*3+2] = 0xb7;
					break;
				case 0x26:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0x77;
					pixels[(i*256+j)*3+2] = 0x63;
					break;
				case 0x27:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0x9b;
					pixels[(i*256+j)*3+2] = 0x3b;
					break;
				case 0x28:
					pixels[(i*256+j)*3+0] = 0xf3;
					pixels[(i*256+j)*3+1] = 0xbf;
					pixels[(i*256+j)*3+2] = 0x3f;
					break;
				case 0x29:
					pixels[(i*256+j)*3+0] = 0x83;
					pixels[(i*256+j)*3+1] = 0xd3;
					pixels[(i*256+j)*3+2] = 0x13;
					break;
				case 0x2a:
					pixels[(i*256+j)*3+0] = 0x4f;
					pixels[(i*256+j)*3+1] = 0xdf;
					pixels[(i*256+j)*3+2] = 0x4b;
					break;
				case 0x2b:
					pixels[(i*256+j)*3+0] = 0x58;
					pixels[(i*256+j)*3+1] = 0xf8;
					pixels[(i*256+j)*3+2] = 0x98;
					break;
				case 0x2c:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0xeb;
					pixels[(i*256+j)*3+2] = 0xdb;
					break;
				case 0x2d:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x2e:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x2f:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x30:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0xff;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x31:
					pixels[(i*256+j)*3+0] = 0xab;
					pixels[(i*256+j)*3+1] = 0xe7;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x32:
					pixels[(i*256+j)*3+0] = 0xc7;
					pixels[(i*256+j)*3+1] = 0xd7;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x33:
					pixels[(i*256+j)*3+0] = 0xd7;
					pixels[(i*256+j)*3+1] = 0xcb;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x34:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0xc7;
					pixels[(i*256+j)*3+2] = 0xff;
					break;
				case 0x35:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0xc7;
					pixels[(i*256+j)*3+2] = 0xdb;
					break;
				case 0x36:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0xbf;
					pixels[(i*256+j)*3+2] = 0xb3;
					break;
				case 0x37:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0xdb;
					pixels[(i*256+j)*3+2] = 0xab;
					break;
				case 0x38:
					pixels[(i*256+j)*3+0] = 0xff;
					pixels[(i*256+j)*3+1] = 0xe7;
					pixels[(i*256+j)*3+2] = 0xa3;
					break;
				case 0x39:
					pixels[(i*256+j)*3+0] = 0xe3;
					pixels[(i*256+j)*3+1] = 0xff;
					pixels[(i*256+j)*3+2] = 0xa3;
					break;
				case 0x3a:
					pixels[(i*256+j)*3+0] = 0xab;
					pixels[(i*256+j)*3+1] = 0xf3;
					pixels[(i*256+j)*3+2] = 0xbf;
					break;
				case 0x3b:
					pixels[(i*256+j)*3+0] = 0xb3;
					pixels[(i*256+j)*3+1] = 0xff;
					pixels[(i*256+j)*3+2] = 0xcf;
					break;
				case 0x3c:
					pixels[(i*256+j)*3+0] = 0x9f;
					pixels[(i*256+j)*3+1] = 0xff;
					pixels[(i*256+j)*3+2] = 0xf3;
					break;
				case 0x3d:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x3e:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x3f:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x40:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x41:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x42:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x43:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x44:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x45:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x46:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x47:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x48:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x49:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x4a:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x4b:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
				case 0x4c:
					pixels[(i*256+j)*3+0] = 0x00;
					pixels[(i*256+j)*3+1] = 0x00;
					pixels[(i*256+j)*3+2] = 0x00;
					break;
			}

		}
	}*/

	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	for (rowCount = 0; rowCount < 240; rowCount++) {
	for (colCount = 0; colCount < 256; colCount++) {
			switch (frame[rowCount][colCount]) {
				case 0x00:
					red = 0x75;
					green = 0x75;
					blue = 0x75;
					break;
				case 0x01:
					red = 0x27;
					green = 0x1b;
					blue = 0x8f;
					break;
				case 0x02:
					red = 0x00;
					green = 0x00;
					blue = 0xab;
					break;
				case 0x03:
					red = 0x47;
					green = 0x00;
					blue = 0x9f;
					break;
				case 0x04:
					red = 0x8f;
					green = 0x00;
					blue = 0x77;
					break;
				case 0x05:
					red = 0xab;
					green = 0x00;
					blue = 0x13;
					break;
				case 0x06:
					red = 0xa7;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x07:
					red = 0x7f;
					green = 0x0b;
					blue = 0x00;
					break;
				case 0x08:
					red = 0x43;
					green = 0x2f;
					blue = 0x00;
					break;
				case 0x09:
					red = 0x00;
					green = 0x47;
					blue = 0x00;
					break;
				case 0x0a:
					red = 0x00;
					green = 0x51;
					blue = 0x00;
					break;
				case 0x0b:
					red = 0x00;
					green = 0x3f;
					blue = 0x17;
					break;
				case 0x0c:
					red = 0x1b;
					green = 0x3f;
					blue = 0x5f;
					break;
				case 0x0d:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x0e:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x0f:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x10:
					red = 0xbc;
					green = 0xbc;
					blue = 0xbc;
					break;
				case 0x11:
					red = 0x00;
					green = 0x73;
					blue = 0xef;
					break;
				case 0x12:
					red = 0x23;
					green = 0x3b;
					blue = 0xef;
					break;
				case 0x13:
					red = 0x83;
					green = 0x00;
					blue = 0xf3;
					break;
				case 0x14:
					red = 0xbf;
					green = 0x00;
					blue = 0xbf;
					break;
				case 0x15:
					red = 0xe7;
					green = 0x00;
					blue = 0x5b;
					break;
				case 0x16:
					red = 0xdb;
					green = 0x2b;
					blue = 0x00;
					break;
				case 0x17:
					red = 0xcb;
					green = 0x4f;
					blue = 0x0f;
					break;
				case 0x18:
					red = 0x8b;
					green = 0x73;
					blue = 0x00;
					break;
				case 0x19:
					red = 0x00;
					green = 0x97;
					blue = 0x00;
					break;
				case 0x1a:
					red = 0x00;
					green = 0xab;
					blue = 0x00;
					break;
				case 0x1b:
					red = 0x00;
					green = 0x93;
					blue = 0x3b;
					break;
				case 0x1c:
					red = 0x00;
					green = 0x83;
					blue = 0x8b;
					break;
				case 0x1d:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x1e:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x1f:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x20:
					red = 0xff;
					green = 0xff;
					blue = 0xff;
					break;
				case 0x21:
					red = 0x3f;
					green = 0xbf;
					blue = 0xff;
					break;
				case 0x22:
					red = 0x5f;
					green = 0x97;
					blue = 0xff;
					break;
				case 0x23:
					red = 0xa7;
					green = 0x8b;
					blue = 0xfd;
					break;
				case 0x24:
					red = 0xf7;
					green = 0x7b;
					blue = 0xff;
					break;
				case 0x25:
					red = 0xff;
					green = 0x77;
					blue = 0xb7;
					break;
				case 0x26:
					red = 0xff;
					green = 0x77;
					blue = 0x63;
					break;
				case 0x27:
					red = 0xff;
					green = 0x9b;
					blue = 0x3b;
					break;
				case 0x28:
					red = 0xf3;
					green = 0xbf;
					blue = 0x3f;
					break;
				case 0x29:
					red = 0x83;
					green = 0xd3;
					blue = 0x13;
					break;
				case 0x2a:
					red = 0x4f;
					green = 0xdf;
					blue = 0x4b;
					break;
				case 0x2b:
					red = 0x58;
					green = 0xf8;
					blue = 0x98;
					break;
				case 0x2c:
					red = 0x00;
					green = 0xeb;
					blue = 0xdb;
					break;
				case 0x2d:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x2e:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x2f:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x30:
					red = 0xff;
					green = 0xff;
					blue = 0xff;
					break;
				case 0x31:
					red = 0xab;
					green = 0xe7;
					blue = 0xff;
					break;
				case 0x32:
					red = 0xc7;
					green = 0xd7;
					blue = 0xff;
					break;
				case 0x33:
					red = 0xd7;
					green = 0xcb;
					blue = 0xff;
					break;
				case 0x34:
					red = 0xff;
					green = 0xc7;
					blue = 0xff;
					break;
				case 0x35:
					red = 0xff;
					green = 0xc7;
					blue = 0xdb;
					break;
				case 0x36:
					red = 0xff;
					green = 0xbf;
					blue = 0xb3;
					break;
				case 0x37:
					red = 0xff;
					green = 0xdb;
					blue = 0xab;
					break;
				case 0x38:
					red = 0xff;
					green = 0xe7;
					blue = 0xa3;
					break;
				case 0x39:
					red = 0xe3;
					green = 0xff;
					blue = 0xa3;
					break;
				case 0x3a:
					red = 0xab;
					green = 0xf3;
					blue = 0xbf;
					break;
				case 0x3b:
					red = 0xb3;
					green = 0xff;
					blue = 0xcf;
					break;
				case 0x3c:
					red = 0x9f;
					green = 0xff;
					blue = 0xf3;
					break;
				case 0x3d:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x3e:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x3f:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x40:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x41:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x42:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x43:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x44:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x45:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x46:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x47:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x48:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x49:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x4a:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x4b:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
				case 0x4c:
					red = 0x00;
					green = 0x00;
					blue = 0x00;
					break;
			}

		for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)	{
			pixels[((rowCount*3+i)*256*9)+(colCount*3+j)*3+0] = red;
			pixels[((rowCount*3+i)*256*9)+(colCount*3+j)*3+1] = green;
			pixels[((rowCount*3+i)*256*9)+(colCount*3+j)*3+2] = blue;
		}

		}
	}

	display_frame(pixels);

	set_ppu_changed(temp);
	return 0;
}

int update_memory() {
	mem_write_cpu(0x2000, registers.controller);	
	mem_write_cpu(0x2001, registers.mask);	
	mem_write_cpu(0x2002, registers.status);	
	mem_write_cpu(0x2003, registers.oamAddress);	
	mem_write_cpu(0x2004, mem_read_oam(registers.oamAddress));	
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
