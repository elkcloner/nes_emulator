#include <stdio.h>
#include "../cpu/cpu.h"
#include "../cpu/cpu_memory.h"

int print_registers() {
	printf("PC\tOP\tSP\tA\tX\tY\tStatus\n");
	printf("%04x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\n",
		get_pc(), mem_read_cpu(get_pc()), get_sp(), get_a(), get_x(), get_y(), get_status());

	return 0;
}

int print_memory() {
	int i = 0;
	int addr;

	for (i = 0; i < MEM_SIZE_CPU; i++) {
		if (mem_read_cpu(i)) {
			i -= i%16;
			addr = i + 16;
			printf("0x%04x: ", i);
			for (; i < addr; i++) {
				printf("%02x ", mem_read_cpu(i));
			}
			i--;
			printf("\n");
		}
	}

	return 0;
}
