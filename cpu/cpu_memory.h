#ifndef __CPU_MEMORY__
#define __CPU_MEMORY__

#include <stdint.h>

#define MEM_SIZE_CPU	0x10000

uint8_t mem_read_cpu(uint16_t);
int mem_write_cpu(uint16_t, uint8_t);

#endif
