#include <stdlib.h>
#include "cpu.h"
#include "instructions.h"
#include "cpu_memory.h"

#define NUM_OPCODES	0x100

typedef enum {ZERO_PAGE, ZERO_PAGE_X, ZERO_PAGE_Y, ABSOLUTE, ABSOLUTE_X,
	ABSOLUTE_Y, INDIRECT, INDIRECT_X, INDIRECT_Y, IMPLIED, ACCUMULATOR,
	IMMEDIATE, RELATIVE} AddrMode;

typedef struct {
	AddrMode mode;
	int length;
	int (*func)(uint16_t);	
} Opcode;

typedef struct {
	uint16_t pc;
	uint8_t sp;
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint8_t status;
} CPUState;

static Opcode *opcodeTable[NUM_OPCODES];
static CPUState state;

int cpu_exec() {
	//TODO
	uint8_t code;
	uint16_t addr;
	uint16_t temp;
	
	// Fetch opcode
	code = mem_read_cpu(state.pc);

	// Get arg based on instruction length and mode
	switch (opcodeTable[code]->length) {
		case 1:
			addr = 0;
			break;
		case 2:
			addr = mem_read_cpu(state.pc + 1);
			break;
		case 3:
			addr = mem_read_cpu(state.pc + 2);
			addr <<= 8;
			addr |= mem_read_cpu(state.pc + 1);
			break;
	}

	switch(opcodeTable[code]->mode) {
		case IMMEDIATE:
			addr = state.pc + 1;
			break;
		case ZERO_PAGE:
			break;
		case ZERO_PAGE_X:
			addr = ((uint8_t) addr + state.x) & 0xff;
			break;
		case ZERO_PAGE_Y:
			addr = ((uint8_t) addr + state.y) & 0xff;
			break;
		case ABSOLUTE:
			break;
		case ABSOLUTE_X:
			addr = addr + state.x;
			break;
		case ABSOLUTE_Y:
			addr = addr + state.y;
			break;
		case INDIRECT:
			temp = mem_read_cpu((0xff00 & addr) | ((uint8_t) (addr + 1)));
			temp <<= 8;
			temp |= mem_read_cpu(addr);
			addr = temp;
			break;
		case INDIRECT_X:
			temp = mem_read_cpu((uint8_t) (addr + state.x + 1));
			temp <<= 8;
			temp |= mem_read_cpu((uint8_t) (addr + state.x));
			addr = temp;
			break;
		case INDIRECT_Y:
			temp = mem_read_cpu((0xff00 & addr) | ((uint8_t) (addr + 1)));
			temp <<= 8;
			temp |= mem_read_cpu(addr);
			addr = temp + state.y;
			break;
		case IMPLIED:
			break;
		case ACCUMULATOR:
			break;
		case RELATIVE:
			addr = state.pc + (int8_t) addr + 2;
	}

	// Increment PC by instruction length
	state.pc += opcodeTable[code]->length;

	// Call instruction function
	opcodeTable[code]->func(addr);

	return 0;
}

int cpu_reset() {
	opcodes_init();
	cpu_init();

	return 0;
}

int cpu_init() {
	state.pc = mem_read_cpu(0xfffd);
	state.pc <<= 8;
	state.pc |= mem_read_cpu(0xfffc);
	state.sp = 0xff;
	state.a = 0;
	state.x = 0;
	state.y = 0;
	state.status = 0;

	return 0;
}

int opcodes_init() {
	int i;
	uint8_t temp;

	for (i = 0; i < NUM_OPCODES; i++) {
		switch(i) {
			case 0x00:
				opcodeTable[0x00] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x00]->mode = IMPLIED;
				opcodeTable[0x00]->length = 1;
				opcodeTable[0x00]->func = &asm_brk;
				break;
			case 0x01:
				opcodeTable[0x01] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x01]->mode = INDIRECT_X;
				opcodeTable[0x01]->length = 2;
				opcodeTable[0x01]->func = &asm_ora;
				break;
			case 0x05:
				opcodeTable[0x05] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x05]->mode = ZERO_PAGE;
				opcodeTable[0x05]->length = 2;
				opcodeTable[0x05]->func = &asm_ora;
				break;
			case 0x06:
				opcodeTable[0x06] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x06]->mode = ZERO_PAGE;
				opcodeTable[0x06]->length = 2;
				opcodeTable[0x06]->func = &asm_asl;
				break;
			case 0x08:
				opcodeTable[0x08] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x08]->mode = IMPLIED;
				opcodeTable[0x08]->length = 1;
				opcodeTable[0x08]->func = &asm_php;
				break;
			case 0x09:
				opcodeTable[0x09] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x09]->mode = IMMEDIATE;
				opcodeTable[0x09]->length = 2;
				opcodeTable[0x09]->func = &asm_ora;
				break;
			case 0x0a:
				opcodeTable[0x0a] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x0a]->mode = ACCUMULATOR;
				opcodeTable[0x0a]->length = 1;
				opcodeTable[0x0a]->func = &asm_asla;
				break;
			case 0x0d:
				opcodeTable[0x0d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x0d]->mode = ABSOLUTE;
				opcodeTable[0x0d]->length = 3;
				opcodeTable[0x0d]->func = &asm_ora;
				break;
			case 0x0e:
				opcodeTable[0x0e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x0e]->mode = ABSOLUTE;
				opcodeTable[0x0e]->length = 3;
				opcodeTable[0x0e]->func = &asm_asl;
				break;
			case 0x10:
				opcodeTable[0x10] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x10]->mode = RELATIVE;
				opcodeTable[0x10]->length = 2;
				opcodeTable[0x10]->func = &asm_bpl;
				break;
			case 0x11:
				opcodeTable[0x11] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x11]->mode = INDIRECT_Y;
				opcodeTable[0x11]->length = 2;
				opcodeTable[0x11]->func = &asm_ora;
				break;
			case 0x15:
				opcodeTable[0x15] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x15]->mode = ZERO_PAGE_X;
				opcodeTable[0x15]->length = 2;
				opcodeTable[0x15]->func = &asm_ora;
				break;
			case 0x16:
				opcodeTable[0x16] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x16]->mode = ZERO_PAGE_X;
				opcodeTable[0x16]->length = 2;
				opcodeTable[0x16]->func = &asm_asl;
				break;
			case 0x18:
				opcodeTable[0x18] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x18]->mode = IMPLIED;
				opcodeTable[0x18]->length = 1;
				opcodeTable[0x18]->func = &asm_clc;
				break;
			case 0x19:
				opcodeTable[0x19] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x19]->mode = ABSOLUTE_Y;
				opcodeTable[0x19]->length = 3;
				opcodeTable[0x19]->func = &asm_ora;
				break;
			case 0x1d:
				opcodeTable[0x1d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x1d]->mode = ABSOLUTE_X;
				opcodeTable[0x1d]->length = 3;
				opcodeTable[0x1d]->func = &asm_ora;
				break;
			case 0x1e:
				opcodeTable[0x1e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x1e]->mode = ABSOLUTE_X;
				opcodeTable[0x1e]->length = 3;
				opcodeTable[0x1e]->func = &asm_asl;
				break;
			case 0x20:
				opcodeTable[0x20] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x20]->mode = ABSOLUTE;
				opcodeTable[0x20]->length = 3;
				opcodeTable[0x20]->func = &asm_jsr;
				break;
			case 0x21:
				opcodeTable[0x21] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x21]->mode = INDIRECT_X;
				opcodeTable[0x21]->length = 2;
				opcodeTable[0x21]->func = &asm_and;
				break;
			case 0x24:
				opcodeTable[0x24] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x24]->mode = ZERO_PAGE;
				opcodeTable[0x24]->length = 2;
				opcodeTable[0x24]->func = &asm_bit;
				break;
			case 0x25:
				opcodeTable[0x25] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x25]->mode = ZERO_PAGE;
				opcodeTable[0x25]->length = 2;
				opcodeTable[0x25]->func = &asm_and;
				break;
			case 0x26:
				opcodeTable[0x26] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x26]->mode = ZERO_PAGE;
				opcodeTable[0x26]->length = 2;
				opcodeTable[0x26]->func = &asm_rol;
				break;
			case 0x28:
				opcodeTable[0x28] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x28]->mode = IMPLIED;
				opcodeTable[0x28]->length = 1;
				opcodeTable[0x28]->func = &asm_plp;
				break;
			case 0x29:
				opcodeTable[0x29] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x29]->mode = IMMEDIATE;
				opcodeTable[0x29]->length = 2;
				opcodeTable[0x29]->func = &asm_and;
				break;
			case 0x2a:
				opcodeTable[0x2a] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x2a]->mode = ACCUMULATOR;
				opcodeTable[0x2a]->length = 1;
				opcodeTable[0x2a]->func = &asm_rola;
				break;
			case 0x2c:
				opcodeTable[0x2c] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x2c]->mode = ABSOLUTE;
				opcodeTable[0x2c]->length = 3;
				opcodeTable[0x2c]->func = &asm_bit;
				break;
			case 0x2d:
				opcodeTable[0x2d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x2d]->mode = ABSOLUTE;
				opcodeTable[0x2d]->length = 3;
				opcodeTable[0x2d]->func = &asm_and;
				break;
			case 0x2e:
				opcodeTable[0x2e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x2e]->mode = ABSOLUTE;
				opcodeTable[0x2e]->length = 3;
				opcodeTable[0x2e]->func = &asm_rol;
				break;
			case 0x30:
				opcodeTable[0x30] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x30]->mode = RELATIVE;
				opcodeTable[0x30]->length = 2;
				opcodeTable[0x30]->func = &asm_bmi;
				break;
			case 0x31:
				opcodeTable[0x31] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x31]->mode = INDIRECT_Y;
				opcodeTable[0x31]->length = 2;
				opcodeTable[0x31]->func = &asm_and;
				break;
			case 0x35:
				opcodeTable[0x35] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x35]->mode = ZERO_PAGE_X;
				opcodeTable[0x35]->length = 2;
				opcodeTable[0x35]->func = &asm_and;
				break;
			case 0x36:
				opcodeTable[0x36] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x36]->mode = ZERO_PAGE_X;
				opcodeTable[0x36]->length = 2;
				opcodeTable[0x36]->func = &asm_rol;
				break;
			case 0x38:
				opcodeTable[0x38] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x38]->mode = IMPLIED;
				opcodeTable[0x38]->length = 1;
				opcodeTable[0x38]->func = &asm_sec;
				break;
			case 0x39:
				opcodeTable[0x39] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x39]->mode = ABSOLUTE_Y;
				opcodeTable[0x39]->length = 3;
				opcodeTable[0x39]->func = &asm_and;
				break;
			case 0x3d:
				opcodeTable[0x3d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x3d]->mode = ABSOLUTE_X;
				opcodeTable[0x3d]->length = 3;
				opcodeTable[0x3d]->func = &asm_and;
				break;
			case 0x3e:
				opcodeTable[0x3e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x3e]->mode = ABSOLUTE_X;
				opcodeTable[0x3e]->length = 3;
				opcodeTable[0x3e]->func = &asm_rol;
				break;
			case 0x40:
				opcodeTable[0x40] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x40]->mode = IMPLIED;
				opcodeTable[0x40]->length = 1;
				opcodeTable[0x40]->func = &asm_rti;
				break;
			case 0x41:
				opcodeTable[0x41] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x41]->mode = INDIRECT_X;
				opcodeTable[0x41]->length = 2;
				opcodeTable[0x41]->func = &asm_eor;
				break;
			case 0x45:
				opcodeTable[0x45] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x45]->mode = ZERO_PAGE;
				opcodeTable[0x45]->length = 2;
				opcodeTable[0x45]->func = &asm_eor;
				break;
			case 0x46:
				opcodeTable[0x46] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x46]->mode = ZERO_PAGE;
				opcodeTable[0x46]->length = 2;
				opcodeTable[0x46]->func = &asm_lsr;
				break;
			case 0x48:
				opcodeTable[0x48] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x48]->mode = IMPLIED;
				opcodeTable[0x48]->length = 1;
				opcodeTable[0x48]->func = &asm_pha;
				break;
			case 0x49:
				opcodeTable[0x49] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x49]->mode = IMMEDIATE;
				opcodeTable[0x49]->length = 2;
				opcodeTable[0x49]->func = &asm_eor;
				break;
			case 0x4a:
				opcodeTable[0x4a] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x4a]->mode = ACCUMULATOR;
				opcodeTable[0x4a]->length = 1;
				opcodeTable[0x4a]->func = &asm_lsra;
				break;
			case 0x4c:
				opcodeTable[0x4c] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x4c]->mode = ABSOLUTE;
				opcodeTable[0x4c]->length = 3;
				opcodeTable[0x4c]->func = &asm_jmp;
				break;
			case 0x4d:
				opcodeTable[0x4d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x4d]->mode = ABSOLUTE;
				opcodeTable[0x4d]->length = 3;
				opcodeTable[0x4d]->func = &asm_eor;
				break;
			case 0x4e:
				opcodeTable[0x4e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x4e]->mode = ABSOLUTE;
				opcodeTable[0x4e]->length = 3;
				opcodeTable[0x4e]->func = &asm_lsr;
				break;
			case 0x50:
				opcodeTable[0x50] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x50]->mode = RELATIVE;
				opcodeTable[0x50]->length = 2;
				opcodeTable[0x50]->func = &asm_bvc;
				break;
			case 0x51:
				opcodeTable[0x51] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x51]->mode = INDIRECT_Y;
				opcodeTable[0x51]->length = 2;
				opcodeTable[0x51]->func = &asm_eor;
				break;
			case 0x55:
				opcodeTable[0x55] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x55]->mode = ZERO_PAGE_X;
				opcodeTable[0x55]->length = 2;
				opcodeTable[0x55]->func = &asm_eor;
				break;
			case 0x56:
				opcodeTable[0x56] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x56]->mode = ZERO_PAGE_X;
				opcodeTable[0x56]->length = 2;
				opcodeTable[0x56]->func = &asm_lsr;
				break;
			case 0x58:
				opcodeTable[0x58] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x58]->mode = IMPLIED;
				opcodeTable[0x58]->length = 1;
				opcodeTable[0x58]->func = &asm_cli;
				break;
			case 0x59:
				opcodeTable[0x59] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x59]->mode = ABSOLUTE_Y;
				opcodeTable[0x59]->length = 3;
				opcodeTable[0x59]->func = &asm_eor;
				break;
			case 0x5d:
				opcodeTable[0x5d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x5d]->mode = ABSOLUTE_X;
				opcodeTable[0x5d]->length = 3;
				opcodeTable[0x5d]->func = &asm_eor;
				break;
			case 0x5e:
				opcodeTable[0x5e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x5e]->mode = ABSOLUTE_X;
				opcodeTable[0x5e]->length = 3;
				opcodeTable[0x5e]->func = &asm_lsr;
				break;
			case 0x60:
				opcodeTable[0x60] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x60]->mode = IMPLIED;
				opcodeTable[0x60]->length = 1;
				opcodeTable[0x60]->func = &asm_rts;
				break;
			case 0x61:
				opcodeTable[0x61] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x61]->mode = INDIRECT_X;
				opcodeTable[0x61]->length = 2;
				opcodeTable[0x61]->func = &asm_adc;
				break;
			case 0x65:
				opcodeTable[0x65] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x65]->mode = ZERO_PAGE;
				opcodeTable[0x65]->length = 2;
				opcodeTable[0x65]->func = &asm_adc;
				break;
			case 0x66:
				opcodeTable[0x66] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x66]->mode = ZERO_PAGE;
				opcodeTable[0x66]->length = 2;
				opcodeTable[0x66]->func = &asm_ror;
				break;
			case 0x68:
				opcodeTable[0x68] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x68]->mode = IMPLIED;
				opcodeTable[0x68]->length = 1;
				opcodeTable[0x68]->func = &asm_pla;
				break;
			case 0x69:
				opcodeTable[0x69] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x69]->mode = IMMEDIATE;
				opcodeTable[0x69]->length = 2;
				opcodeTable[0x69]->func = &asm_adc;
				break;
			case 0x6a:
				opcodeTable[0x6a] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x6a]->mode = ACCUMULATOR;
				opcodeTable[0x6a]->length = 1;
				opcodeTable[0x6a]->func = &asm_rora;
				break;
			case 0x6c:
				opcodeTable[0x6c] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x6c]->mode = INDIRECT;
				opcodeTable[0x6c]->length = 3;
				opcodeTable[0x6c]->func = &asm_jmp;
				break;
			case 0x6d:
				opcodeTable[0x6d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x6d]->mode = ABSOLUTE;
				opcodeTable[0x6d]->length = 3;
				opcodeTable[0x6d]->func = &asm_adc;
				break;
			case 0x6e:
				opcodeTable[0x6e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x6e]->mode = ABSOLUTE;
				opcodeTable[0x6e]->length = 3;
				opcodeTable[0x6e]->func = &asm_ror;
				break;
			case 0x70:
				opcodeTable[0x70] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x70]->mode = RELATIVE;
				opcodeTable[0x70]->length = 2;
				opcodeTable[0x70]->func = &asm_bvs;
				break;
			case 0x71:
				opcodeTable[0x71] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x71]->mode = INDIRECT_Y;
				opcodeTable[0x71]->length = 2;
				opcodeTable[0x71]->func = &asm_adc;
				break;
			case 0x75:
				opcodeTable[0x75] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x75]->mode = ZERO_PAGE_X;
				opcodeTable[0x75]->length = 2;
				opcodeTable[0x75]->func = &asm_adc;
				break;
			case 0x76:
				opcodeTable[0x76] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x76]->mode = ZERO_PAGE_X;
				opcodeTable[0x76]->length = 2;
				opcodeTable[0x76]->func = &asm_ror;
				break;
			case 0x78:
				opcodeTable[0x78] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x78]->mode = IMPLIED;
				opcodeTable[0x78]->length = 1;
				opcodeTable[0x78]->func = &asm_sei;
				break;
			case 0x79:
				opcodeTable[0x79] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x79]->mode = ABSOLUTE_Y;
				opcodeTable[0x79]->length = 3;
				opcodeTable[0x79]->func = &asm_adc;
				break;
			case 0x7d:
				opcodeTable[0x7d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x7d]->mode = ABSOLUTE_X;
				opcodeTable[0x7d]->length = 3;
				opcodeTable[0x7d]->func = &asm_adc;
				break;
			case 0x7e:
				opcodeTable[0x7e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x7e]->mode = ABSOLUTE_X;
				opcodeTable[0x7e]->length = 3;
				opcodeTable[0x7e]->func = &asm_ror;
				break;
			case 0x81:
				opcodeTable[0x81] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x81]->mode = INDIRECT_X;
				opcodeTable[0x81]->length = 2;
				opcodeTable[0x81]->func = &asm_sta;
				break;
			case 0x84:
				opcodeTable[0x84] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x84]->mode = ZERO_PAGE;
				opcodeTable[0x84]->length = 2;
				opcodeTable[0x84]->func = &asm_sty;
				break;
			case 0x85:
				opcodeTable[0x85] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x85]->mode = ZERO_PAGE;
				opcodeTable[0x85]->length = 2;
				opcodeTable[0x85]->func = &asm_sta;
				break;
			case 0x86:
				opcodeTable[0x86] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x86]->mode = ZERO_PAGE;
				opcodeTable[0x86]->length = 2;
				opcodeTable[0x86]->func = &asm_stx;
				break;
			case 0x88:
				opcodeTable[0x88] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x88]->mode = IMPLIED;
				opcodeTable[0x88]->length = 1;
				opcodeTable[0x88]->func = &asm_dey;
				break;
			case 0x8a:
				opcodeTable[0x8a] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x8a]->mode = IMPLIED;
				opcodeTable[0x8a]->length = 1;
				opcodeTable[0x8a]->func = &asm_txa;
				break;
			case 0x8c:
				opcodeTable[0x8c] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x8c]->mode = ABSOLUTE;
				opcodeTable[0x8c]->length = 3;
				opcodeTable[0x8c]->func = &asm_sty;
				break;
			case 0x8d:
				opcodeTable[0x8d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x8d]->mode = ABSOLUTE;
				opcodeTable[0x8d]->length = 3;
				opcodeTable[0x8d]->func = &asm_sta;
				break;
			case 0x8e:
				opcodeTable[0x8e] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x8e]->mode = ABSOLUTE;
				opcodeTable[0x8e]->length = 3;
				opcodeTable[0x8e]->func = &asm_stx;
				break;
			case 0x90:
				opcodeTable[0x90] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x90]->mode = RELATIVE;
				opcodeTable[0x90]->length = 2;
				opcodeTable[0x90]->func = &asm_bcc;
				break;
			case 0x91:
				opcodeTable[0x91] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x91]->mode = INDIRECT_Y;
				opcodeTable[0x91]->length = 2;
				opcodeTable[0x91]->func = &asm_sta;
				break;
			case 0x94:
				opcodeTable[0x94] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x94]->mode = ZERO_PAGE_X;
				opcodeTable[0x94]->length = 2;
				opcodeTable[0x94]->func = &asm_sty;
				break;
			case 0x95:
				opcodeTable[0x95] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x95]->mode = ZERO_PAGE_X;
				opcodeTable[0x95]->length = 2;
				opcodeTable[0x95]->func = &asm_sta;
				break;
			case 0x96:
				opcodeTable[0x96] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x96]->mode = ZERO_PAGE_Y;
				opcodeTable[0x96]->length = 2;
				opcodeTable[0x96]->func = &asm_stx;
				break;
			case 0x98:
				opcodeTable[0x98] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x98]->mode = IMPLIED;
				opcodeTable[0x98]->length = 1;
				opcodeTable[0x98]->func = &asm_tya;
				break;
			case 0x99:
				opcodeTable[0x99] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x99]->mode = ABSOLUTE_Y;
				opcodeTable[0x99]->length = 3;
				opcodeTable[0x99]->func = &asm_sta;
				break;
			case 0x9a:
				opcodeTable[0x9a] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x9a]->mode = IMPLIED;
				opcodeTable[0x9a]->length = 1;
				opcodeTable[0x9a]->func = &asm_txs;
				break;
			case 0x9d:
				opcodeTable[0x9d] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0x9d]->mode = ABSOLUTE_X;
				opcodeTable[0x9d]->length = 3;
				opcodeTable[0x9d]->func = &asm_sta;
				break;
			case 0xa0:
				opcodeTable[0xa0] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa0]->mode = IMMEDIATE;
				opcodeTable[0xa0]->length = 2;
				opcodeTable[0xa0]->func = &asm_ldy;
				break;
			case 0xa1:
				opcodeTable[0xa1] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa1]->mode = INDIRECT_X;
				opcodeTable[0xa1]->length = 2;
				opcodeTable[0xa1]->func = &asm_lda;
				break;
			case 0xa2:
				opcodeTable[0xa2] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa2]->mode = IMMEDIATE;
				opcodeTable[0xa2]->length = 2;
				opcodeTable[0xa2]->func = &asm_ldx;
				break;
			case 0xa4:
				opcodeTable[0xa4] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa4]->mode = ZERO_PAGE;
				opcodeTable[0xa4]->length = 2;
				opcodeTable[0xa4]->func = &asm_ldy;
				break;
			case 0xa5:
				opcodeTable[0xa5] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa5]->mode = ZERO_PAGE;
				opcodeTable[0xa5]->length = 2;
				opcodeTable[0xa5]->func = &asm_lda;
				break;
			case 0xa6:
				opcodeTable[0xa6] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa6]->mode = ZERO_PAGE;
				opcodeTable[0xa6]->length = 2;
				opcodeTable[0xa6]->func = &asm_ldx;
				break;
			case 0xa8:
				opcodeTable[0xa8] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa8]->mode = IMPLIED;
				opcodeTable[0xa8]->length = 1;
				opcodeTable[0xa8]->func = &asm_tay;
				break;
			case 0xa9:
				opcodeTable[0xa9] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xa9]->mode = IMMEDIATE;
				opcodeTable[0xa9]->length = 2;
				opcodeTable[0xa9]->func = &asm_lda;
				break;
			case 0xaa:
				opcodeTable[0xaa] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xaa]->mode = IMPLIED;
				opcodeTable[0xaa]->length = 1;
				opcodeTable[0xaa]->func = &asm_tax;
				break;
			case 0xac:
				opcodeTable[0xac] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xac]->mode = ABSOLUTE;
				opcodeTable[0xac]->length = 3;
				opcodeTable[0xac]->func = &asm_ldy;
				break;
			case 0xad:
				opcodeTable[0xad] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xad]->mode = ABSOLUTE;
				opcodeTable[0xad]->length = 3;
				opcodeTable[0xad]->func = &asm_lda;
				break;
			case 0xae:
				opcodeTable[0xae] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xae]->mode = ABSOLUTE;
				opcodeTable[0xae]->length = 3;
				opcodeTable[0xae]->func = &asm_ldx;
				break;
			case 0xb0:
				opcodeTable[0xb0] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xb0]->mode = RELATIVE;
				opcodeTable[0xb0]->length = 2;
				opcodeTable[0xb0]->func = &asm_bcs;
				break;
			case 0xb1:
				opcodeTable[0xb1] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xb1]->mode = INDIRECT_Y;
				opcodeTable[0xb1]->length = 2;
				opcodeTable[0xb1]->func = &asm_lda;
				break;
			case 0xb4:
				opcodeTable[0xb4] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xb4]->mode = ZERO_PAGE_X;
				opcodeTable[0xb4]->length = 2;
				opcodeTable[0xb4]->func = &asm_ldy;
				break;
			case 0xb5:
				opcodeTable[0xb5] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xb5]->mode = ZERO_PAGE_X;
				opcodeTable[0xb5]->length = 2;
				opcodeTable[0xb5]->func = &asm_lda;
				break;
			case 0xb6:
				opcodeTable[0xb6] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xb6]->mode = ZERO_PAGE_Y;
				opcodeTable[0xb6]->length = 2;
				opcodeTable[0xb6]->func = &asm_ldx;
				break;
			case 0xb8:
				opcodeTable[0xb8] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xb8]->mode = IMPLIED;
				opcodeTable[0xb8]->length = 1;
				opcodeTable[0xb8]->func = &asm_clv;
				break;
			case 0xb9:
				opcodeTable[0xb9] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xb9]->mode = ABSOLUTE_Y;
				opcodeTable[0xb9]->length = 3;
				opcodeTable[0xb9]->func = &asm_lda;
				break;
			case 0xba:
				opcodeTable[0xba] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xba]->mode = IMPLIED;
				opcodeTable[0xba]->length = 1;
				opcodeTable[0xba]->func = &asm_tsx;
				break;
			case 0xbc:
				opcodeTable[0xbc] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xbc]->mode = ABSOLUTE_X;
				opcodeTable[0xbc]->length = 3;
				opcodeTable[0xbc]->func = &asm_ldy;
				break;
			case 0xbd:
				opcodeTable[0xbd] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xbd]->mode = ABSOLUTE_X;
				opcodeTable[0xbd]->length = 3;
				opcodeTable[0xbd]->func = &asm_lda;
				break;
			case 0xbe:
				opcodeTable[0xbe] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xbe]->mode = ABSOLUTE_Y;
				opcodeTable[0xbe]->length = 3;
				opcodeTable[0xbe]->func = &asm_ldx;
				break;
			case 0xc0:
				opcodeTable[0xc0] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xc0]->mode = IMMEDIATE;
				opcodeTable[0xc0]->length = 2;
				opcodeTable[0xc0]->func = &asm_cpy;
				break;
			case 0xc1:
				opcodeTable[0xc1] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xc1]->mode = INDIRECT_X;
				opcodeTable[0xc1]->length = 2;
				opcodeTable[0xc1]->func = &asm_cmp;
				break;
			case 0xc4:
				opcodeTable[0xc4] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xc4]->mode = ZERO_PAGE;
				opcodeTable[0xc4]->length = 2;
				opcodeTable[0xc4]->func = &asm_cpy;
				break;
			case 0xc5:
				opcodeTable[0xc5] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xc5]->mode = ZERO_PAGE;
				opcodeTable[0xc5]->length = 2;
				opcodeTable[0xc5]->func = &asm_cmp;
				break;
			case 0xc6:
				opcodeTable[0xc6] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xc6]->mode = ZERO_PAGE;
				opcodeTable[0xc6]->length = 2;
				opcodeTable[0xc6]->func = &asm_dec;
				break;
			case 0xc8:
				opcodeTable[0xc8] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xc8]->mode = IMPLIED;
				opcodeTable[0xc8]->length = 1;
				opcodeTable[0xc8]->func = &asm_iny;
				break;
			case 0xc9:
				opcodeTable[0xc9] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xc9]->mode = IMMEDIATE;
				opcodeTable[0xc9]->length = 2;
				opcodeTable[0xc9]->func = &asm_cmp;
				break;
			case 0xca:
				opcodeTable[0xca] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xca]->mode = IMPLIED;
				opcodeTable[0xca]->length = 1;
				opcodeTable[0xca]->func = &asm_dex;
				break;
			case 0xcc:
				opcodeTable[0xcc] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xcc]->mode = ABSOLUTE;
				opcodeTable[0xcc]->length = 3;
				opcodeTable[0xcc]->func = &asm_cpy;
				break;
			case 0xcd:
				opcodeTable[0xcd] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xcd]->mode = ABSOLUTE;
				opcodeTable[0xcd]->length = 3;
				opcodeTable[0xcd]->func = &asm_cmp;
				break;
			case 0xce:
				opcodeTable[0xce] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xce]->mode = ABSOLUTE;
				opcodeTable[0xce]->length = 3;
				opcodeTable[0xce]->func = &asm_dec;
				break;
			case 0xd0:
				opcodeTable[0xd0] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xd0]->mode = RELATIVE;
				opcodeTable[0xd0]->length = 2;
				opcodeTable[0xd0]->func = &asm_bne;
				break;
			case 0xd1:
				opcodeTable[0xd1] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xd1]->mode = INDIRECT_Y;
				opcodeTable[0xd1]->length = 2;
				opcodeTable[0xd1]->func = &asm_cmp;
				break;
			case 0xd5:
				opcodeTable[0xd5] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xd5]->mode = ZERO_PAGE_X;
				opcodeTable[0xd5]->length = 2;
				opcodeTable[0xd5]->func = &asm_cmp;
				break;
			case 0xd6:
				opcodeTable[0xd6] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xd6]->mode = ZERO_PAGE_X;
				opcodeTable[0xd6]->length = 2;
				opcodeTable[0xd6]->func = &asm_dec;
				break;
			case 0xd8:
				opcodeTable[0xd8] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xd8]->mode = IMPLIED;
				opcodeTable[0xd8]->length = 1;
				opcodeTable[0xd8]->func = &asm_cld;
				break;
			case 0xd9:
				opcodeTable[0xd9] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xd9]->mode = ABSOLUTE_Y;
				opcodeTable[0xd9]->length = 3;
				opcodeTable[0xd9]->func = &asm_cmp;
				break;
			case 0xdd:
				opcodeTable[0xdd] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xdd]->mode = ABSOLUTE_X;
				opcodeTable[0xdd]->length = 3;
				opcodeTable[0xdd]->func = &asm_cmp;
				break;
			case 0xde:
				opcodeTable[0xde] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xde]->mode = ABSOLUTE_X;
				opcodeTable[0xde]->length = 3;
				opcodeTable[0xde]->func = &asm_dec;
				break;
			case 0xe0:
				opcodeTable[0xe0] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xe0]->mode = IMMEDIATE;
				opcodeTable[0xe0]->length = 2;
				opcodeTable[0xe0]->func = &asm_cpx;
				break;
			case 0xe1:
				opcodeTable[0xe1] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xe1]->mode = INDIRECT_X;
				opcodeTable[0xe1]->length = 2;
				opcodeTable[0xe1]->func = &asm_sbc;
				break;
			case 0xe4:
				opcodeTable[0xe4] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xe4]->mode = ZERO_PAGE;
				opcodeTable[0xe4]->length = 2;
				opcodeTable[0xe4]->func = &asm_cpx;
				break;
			case 0xe5:
				opcodeTable[0xe5] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xe5]->mode = ZERO_PAGE;
				opcodeTable[0xe5]->length = 2;
				opcodeTable[0xe5]->func = &asm_sbc;
				break;
			case 0xe6:
				opcodeTable[0xe6] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xe6]->mode = ZERO_PAGE;
				opcodeTable[0xe6]->length = 2;
				opcodeTable[0xe6]->func = &asm_inc;
				break;
			case 0xe8:
				opcodeTable[0xe8] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xe8]->mode = IMPLIED;
				opcodeTable[0xe8]->length = 1;
				opcodeTable[0xe8]->func = &asm_inx;
				break;
			case 0xe9:
				opcodeTable[0xe9] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xe9]->mode = IMMEDIATE;
				opcodeTable[0xe9]->length = 2;
				opcodeTable[0xe9]->func = &asm_sbc;
				break;
			case 0xea:
				opcodeTable[0xea] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xea]->mode = IMPLIED;
				opcodeTable[0xea]->length = 1;
				opcodeTable[0xea]->func = &asm_nop;
				break;
			case 0xec:
				opcodeTable[0xec] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xec]->mode = ABSOLUTE;
				opcodeTable[0xec]->length = 3;
				opcodeTable[0xec]->func = &asm_cpx;
				break;
			case 0xed:
				opcodeTable[0xed] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xed]->mode = ABSOLUTE;
				opcodeTable[0xed]->length = 3;
				opcodeTable[0xed]->func = &asm_sbc;
				break;
			case 0xee:
				opcodeTable[0xee] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xee]->mode = ABSOLUTE;
				opcodeTable[0xee]->length = 3;
				opcodeTable[0xee]->func = &asm_inc;
				break;
			case 0xf0:
				opcodeTable[0xf0] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xf0]->mode = RELATIVE;
				opcodeTable[0xf0]->length = 2;
				opcodeTable[0xf0]->func = &asm_beq;
				break;
			case 0xf1:
				opcodeTable[0xf1] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xf1]->mode = INDIRECT_Y;
				opcodeTable[0xf1]->length = 2;
				opcodeTable[0xf1]->func = &asm_sbc;
				break;
			case 0xf5:
				opcodeTable[0xf5] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xf5]->mode = ZERO_PAGE_X;
				opcodeTable[0xf5]->length = 2;
				opcodeTable[0xf5]->func = &asm_sbc;
				break;
			case 0xf6:
				opcodeTable[0xf6] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xf6]->mode = ZERO_PAGE_X;
				opcodeTable[0xf6]->length = 2;
				opcodeTable[0xf6]->func = &asm_inc;
				break;
			case 0xf8:
				opcodeTable[0xf8] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xf8]->mode = IMPLIED;
				opcodeTable[0xf8]->length = 1;
				opcodeTable[0xf8]->func = &asm_sed;
				break;
			case 0xf9:
				opcodeTable[0xf9] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xf9]->mode = ABSOLUTE_Y;
				opcodeTable[0xf9]->length = 3;
				opcodeTable[0xf9]->func = &asm_sbc;
				break;
			case 0xfd:
				opcodeTable[0xfd] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xfd]->mode = ABSOLUTE_X;
				opcodeTable[0xfd]->length = 3;
				opcodeTable[0xfd]->func = &asm_sbc;
				break;
			case 0xfe:
				opcodeTable[0xfe] = (Opcode *) malloc(sizeof(Opcode));
				opcodeTable[0xfe]->mode = ABSOLUTE_X;
				opcodeTable[0xfe]->length = 3;
				opcodeTable[0xfe]->func = &asm_inc;
				break;
			default:
				temp = i & 0xf;	
				switch (temp) {
					case 0xa:
						// one byte
						opcodeTable[i] = (Opcode *) malloc(sizeof(Opcode));
						opcodeTable[i]->mode = IMPLIED;
						opcodeTable[i]->length = 1;
						opcodeTable[i]->func = &asm_nop;
						break;
					case 0x0:
					case 0x2:
					case 0x4:
					case 0x9:
						// two bytes
						opcodeTable[i] = (Opcode *) malloc(sizeof(Opcode));
						opcodeTable[i]->mode = IMMEDIATE;
						opcodeTable[i]->length = 2;
						opcodeTable[i]->func = &asm_nop;
						break;
					case 0xc:
						// three bytes	
						opcodeTable[i] = (Opcode *) malloc(sizeof(Opcode));
						opcodeTable[i]->mode = ABSOLUTE;
						opcodeTable[i]->length = 3;
						opcodeTable[i]->func = &asm_nop;
						break;
					default:
						opcodeTable[i] = NULL;
				}
		}
	}

	return 0;
}

int opcodes_clean() {
	int i;

	for (i = 0; i < NUM_OPCODES; i++)
		if (opcodeTable[i] != NULL)
			free(opcodeTable[i]);

	return 0;
}

uint16_t get_pc() {
	return state.pc;
}

uint8_t get_sp() {
	return state.sp;
}

uint8_t get_a() {
	return state.a;
}

uint8_t get_x() {
	return state.x;
}

uint8_t get_y() {
	return state.y;
}

uint8_t get_status() {
	return (0xef & state.status) | 0x20;
}

int set_pc(uint16_t value) {
	state.pc = value;
	return 0;
}

int set_sp(uint8_t value) {
	state.sp = value;
	return 0;
}

int set_a(uint8_t value) {
	state.a = value;
	return 0;
}

int set_x(uint8_t value) {
	state.x = value;
	return 0;
}

int set_y(uint8_t value) {
	state.y = value;
	return 0;
}

int set_status(uint8_t value) {
	state.status = value;
	return 0;
}

bool get_nFlag() {
	if (0x80 & state.status)
		return true;
	else
		return false;	
}

bool get_vFlag() {
	if (0x40 & state.status)
		return true;
	else
		return false;	
}

bool get_bFlag() {
	if (0x10 & state.status)
		return true;
	else
		return false;	
}

bool get_dFlag() {
	if (0x08 & state.status)
		return true;
	else
		return false;	
}

bool get_iFlag() {
	if (0x04 & state.status)
		return true;
	else
		return false;	
}

bool get_zFlag() {
	if (0x02 & state.status)
		return true;
	else
		return false;	
}

bool get_cFlag() {
	if (0x01 & state.status)
		return true;
	else
		return false;	
}

int set_nFlag() {
	state.status |= 0x80;
	return 0;
}

int set_vFlag() {
	state.status |= 0x40;
	return 0;
}

int set_bFlag() {
	state.status |= 0x10;
	return 0;
}

int set_dFlag() {
	state.status |= 0x08;
	return 0;
}

int set_iFlag() {
	state.status |= 0x04;
	return 0;
}

int set_zFlag() {
	state.status |= 0x02;
	return 0;
}

int set_cFlag() {
	state.status |= 0x01;
	return 0;
}

int clr_nFlag() {
	state.status &= ~0x80;
	return 0;
}

int clr_vFlag() {
	state.status &= ~0x40;
	return 0;
}

int clr_bFlag() {
	state.status &= ~0x10;
	return 0;
}

int clr_dFlag() {
	state.status &= ~0x08;
	return 0;
}

int clr_iFlag() {
	state.status &= ~0x04;
	return 0;
}

int clr_zFlag() {
	state.status &= ~0x02;
	return 0;
}

int clr_cFlag() {
	state.status &= ~0x01;
	return 0;
}

