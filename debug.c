#include <stdio.h>
#include "cpu.h"
#include "memory.h"

int print_registers() {
	printf("PC\tOP\tSP\tA\tX\tY\tStatus\n");
	printf("%04x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\n",
		get_pc(), mem_read(get_pc()), get_sp(), get_a(), get_x(), get_y(), get_status());
}

int print_memory() {
	int i = 0;
	int addr;

	for (i = 0; i < MEM_SIZE; i++) {
		if (mem_read(i)) {
			i -= i%16;
			addr = i + 16;
			printf("0x%04x: ", i);
			for (i; i < addr; i++) {
				printf("%02x ", mem_read(i));
			}
			i--;
			printf("\n");
		}
	}
}
