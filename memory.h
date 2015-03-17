#ifndef __MEMORY__
#define __MEMORY__

#include <stdint.h>

#define MEM_SIZE	0x10000

uint8_t mem_read(uint16_t);
int mem_write(uint16_t, uint8_t);

uint8_t mem_read_ppu(uint16_t);
int mem_write_ppu(uint16_t, uint8_t);

int mem_init();

int load_rom(char *);

#endif
