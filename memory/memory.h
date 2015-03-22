#ifndef __MEMORY__
#define __MEMORY__

#include <stdint.h>

typedef enum {NONE, WRITE_2000, WRITE_2001, READ_2002,
	WRITE_2003, WRITE_2004, READ_2004, WRITE_2005, WRITE_2006, WRITE_2007,
	READ_2007, WRITE_4014} ppuChange;

int mem_write_cpu(uint16_t, uint8_t);
uint8_t mem_read_cpu(uint16_t);

int mem_write_ppu(uint16_t, uint8_t);
uint8_t mem_read_ppu(uint16_t);

int mem_write_oam(uint16_t, uint8_t);
uint8_t mem_read_oam(uint16_t);

ppuChange get_ppu_changed();
int set_ppu_changed(ppuChange);

int load_rom(char *);

#endif
