#ifndef __PPU_MEMORY__
#define __PPU_MEMORY__

#include <stdint.h>

#define MEM_SIZE_PPU	0x4000

uint8_t mem_read_ppu(uint16_t);
int mem_write_ppu(uint16_t, uint8_t);

uint8_t read_sprite_mem(uint8_t);
int write_sprite_mem(uint8_t, uint8_t);

#endif
