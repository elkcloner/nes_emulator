#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "memory.h"
#include "../misc/display.h"

#define HEADER_SIZE		0x10
#define TRAINER_SIZE	0x200
#define PRG_BANK_SIZE	0x4000
#define CHR_BANK_SIZE	0x2000
#define MAX_NUM_PRG		16	
#define MAX_NUM_CHR		4
#define SRAM_SIZE		0x2000
#define EXPAC_ROM_SIZE	0x1fe0
#define PPU_REG_SIZE	0x20
#define PPA_REG_SIZE	0x08
#define RAM_SIZE		0x800
#define PAL_RAM_SIZE	0x20
#define MAX_NUM_NT		2
#define N_TABLE_SIZE	0x800
#define P_TABLE_SIZE	0x2000
#define OAM_SIZE		0x100

static ppuChange change = NONE;

static uint8_t header[HEADER_SIZE];
static uint8_t trainer[TRAINER_SIZE];

static uint8_t prgBanks[MAX_NUM_PRG][PRG_BANK_SIZE];
static uint8_t chrBanks[MAX_NUM_CHR][CHR_BANK_SIZE];
static uint8_t sram[SRAM_SIZE];
static uint8_t expansionROM[EXPAC_ROM_SIZE];
static uint8_t ppaRegs[PPA_REG_SIZE];
static uint8_t ppuRegs[PPU_REG_SIZE];
static uint8_t ram[RAM_SIZE];

static uint8_t paletteRam[PAL_RAM_SIZE];
static uint8_t nameTables[MAX_NUM_NT][N_TABLE_SIZE];

static uint8_t oam[OAM_SIZE];

static uint8_t mapper;
static uint8_t numPRG, numCHR;
//static int numPRGRAM;
static bool verticalMirroring;
//static bool batterySRAM;
//static bool fourScreen;
static bool trainerPresent;

int mem_write_cpu(uint16_t address, uint8_t value) {
	// TODO ppu status change mirroring support
	switch (address) {
		case 0x2000:
			set_ppu_changed(WRITE_2000);
			break;
		case 0x2001:
			set_ppu_changed(WRITE_2001);
			break;
		case 0x2003:
			set_ppu_changed(WRITE_2003);
			break;
		case 0x2004:
			set_ppu_changed(WRITE_2004);
			break;
		case 0x2005:
			set_ppu_changed(WRITE_2005);
			break;
		case 0x2006:
			set_ppu_changed(WRITE_2006);
			break;
		case 0x2007:
			set_ppu_changed(WRITE_2007);
			break;
		case 0x4014:
			set_ppu_changed(WRITE_4014);
			break;
	}

	if (0xc000 <= address) {
		// Upper PRG bank
		prgBanks[numPRG-1][address-0xc000] = value;	
	} else if (0x8000 <= address) {
		// Lower PRG bank
		prgBanks[0][address-0x8000] = value;	
	} else if (0x6000 <= address) {
		// SRAM
		sram[address-0x6000] = value;
	} else if (0x4020 <= address) {
		// Expansion ROM
		expansionROM[address-0x4020] = value;
	} else if (0x4000 <= address) {
		// PPA and I/O register
		if (address == 0x4016) {
			controller_write(value);
		}
		ppaRegs[address-0x4000] = value;		
	} else if (0x2008 <= address) {
		// Mirror into 0x2000-0x2007
		ppuRegs[address%8] = value;
	} else if (0x2000 <= address) {
		// PPU registers
		ppuRegs[address-0x2000] = value;
	} else if (0x0800 <= address) {
		// Mirror into 0x0000-0x07ff
		ram[address%0x8000] = value;
	} else {
		// System RAM
		ram[address] = value;
	}

	return 0;
}

uint8_t mem_read_cpu(uint16_t address) {
	// TODO mirroring in status change
	switch (address) {
		case 0x2002:
			set_ppu_changed(READ_2002);
			break;
		case 0x2004:
			set_ppu_changed(READ_2004);
			break;
		case 0x2007:
			set_ppu_changed(READ_2007);
			break;
	}

	if (0xc000 <= address) {
		// Upper PRG bank
		return prgBanks[numPRG-1][address-0xc000];	
	} else if (0x8000 <= address) {
		// Lower PRG bank
		return prgBanks[0][address-0x8000];	
	} else if (0x6000 <= address) {
		// SRAM
		return sram[address-0x6000];
	} else if (0x4020 <= address) {
		// Expansion ROM
		return expansionROM[address-0x4020];
	} else if (0x4000 <= address) {
		// PPA and I/O register
		if (address == 0x4016) {
			return ppaRegs[0x16] = controller_read(address);
		}
		return ppaRegs[address-0x4000];		
	} else if (0x2008 <= address) {
		// Mirror into 0x2000-0x2007
		return ppuRegs[address%8];
	} else if (0x2000 <= address) {
		// PPU registers
		return ppuRegs[address-0x2000];
	} else if (0x0800 <= address) {
		// Mirror into 0x0000-0x07ff
		return ram[address%0x0800];
	} else {
		// System RAM
		return ram[address];
	}
}

int mem_write_ppu(uint16_t address, uint8_t value) {
	if (0x4000 <= address) {
		// Mirroring into 0x0000-0x3fff
		mem_write_ppu(address%4000, value);
	} else if (0x3f20 <= address) {
		// Mirror into 0x3f00-0x3f1f
		paletteRam[address%0x20] = value;
	} else if (0x3f00 <= address) {
		// Palette RAM
		paletteRam[address-0x3f00] = value;
	} else if (0x3000 <= address) {
		// Mirror into 0x2000-0x2eff
		mem_write_ppu(address-0x1000, value);
	} else if (0x2c00 <= address) {
		// Nametable 3
		nameTables[1][address-0x2c00] = value;
	} else if (0x2800 <= address) {
		// Nametable 2
		if (verticalMirroring) {
			nameTables[0][address-0x2800] = value;	
		} else {
			nameTables[1][address-0x2800] = value;	
		}
	} else if (0x2400 <= address) {
		// Nametable 1
		if (verticalMirroring) {
			nameTables[1][address-0x2400] = value;
		} else {
			nameTables[0][address-0x2400] = value;
		}
	} else if (0x2000 <= address) {
		// Nametable 0
		nameTables[0][address-0x2000] = value;
	} else {
		// Pattern tables
		chrBanks[0][address] = value;
	}

	return 0;
}

uint8_t mem_read_ppu(uint16_t address) {
	if (0x4000 <= address) {
		// Mirroring into 0x0000-0x3fff
		return mem_read_ppu(address%4000);
	} else if (0x3f20 <= address) {
		// Mirror into 0x3f00-0x3f1f
		return paletteRam[address%0x20];
	} else if (0x3f00 <= address) {
		// Palette RAM
		return paletteRam[address-0x3f00];
	} else if (0x3000 <= address) {
		// Mirror into 0x2000-0x2eff
		return mem_read_ppu(address-0x1000);
	} else if (0x2c00 <= address) {
		// Nametable 3
		return nameTables[1][address-0x2c00];
	} else if (0x2800 <= address) {
		// Nametable 2
		if (verticalMirroring) {
			return nameTables[0][address-0x2800];	
		} else {
			return nameTables[1][address-0x2800];	
		}
	} else if (0x2400 <= address) {
		// Nametable 1
		if (verticalMirroring) {
			return nameTables[1][address-0x2400];
		} else {
			return nameTables[0][address-0x2400];
		}
	} else if (0x2000 <= address) {
		// Nametable 0
		return nameTables[0][address-0x2000];
	} else {
		// Pattern tables
		return chrBanks[0][address];
	}
}

int mem_write_oam(uint16_t address, uint8_t value) {
	oam[address] = value;

	return 0;
}

uint8_t mem_read_oam(uint16_t address) {
	return oam[address];;
}

// TODO all this ppu changed stuff
ppuChange get_ppu_changed() {
	return change;
}

int set_ppu_changed(ppuChange newChange) {
	change = newChange;
	return 0;
}

int load_rom(char *filename) {
	FILE *fp;
	int i;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Failed to open nes file %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// Read header	
	fread(header, 1, HEADER_SIZE, fp);
	
	// Get important header values
	mapper = header[6] >> 4;
	mapper |= 0xf0 & header[7];

	numPRG = header[4];
	numCHR = header[5];

	if (0x01 & header[6]) {
		verticalMirroring = true;
	} else {
		verticalMirroring = false;
	}

	if (0x04 & header[6]) {
		trainerPresent = true;
	} else {
		trainerPresent = false;
	}

	// Read trainer, if present
	if (trainerPresent) {
		fread(trainer, 1, TRAINER_SIZE, fp);
	}

	// Read the PRG banks
	for (i = 0; i < numPRG; i++) {
		fread(&(prgBanks[i]), 1, PRG_BANK_SIZE, fp);
	}	

	// Read the CHR banks
	for (i = 0; i < numCHR; i++) {
		fread(&(chrBanks[i]), 1, CHR_BANK_SIZE, fp);
	}

	return 0;
}
