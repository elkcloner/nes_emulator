#ifndef __CPU__
#define __CPU__

#include <stdint.h>
#include <stdbool.h>

int cpu_exec();

int cpu_init();
int opcodes_init();
int opcodes_clean();

uint16_t get_pc();
uint8_t get_sp();
uint8_t get_a();
uint8_t get_x();
uint8_t get_y();
uint8_t get_status();

int set_pc(uint16_t);
int set_sp(uint8_t);
int set_a(uint8_t);
int set_x(uint8_t);
int set_y(uint8_t);
int set_status(uint8_t);

bool get_nFlag();
bool get_vFlag();
bool get_bFlag();
bool get_dFlag();
bool get_iFlag();
bool get_zFlag();
bool get_cFlag();

int set_nFlag();
int set_vFlag();
int set_bFlag();
int set_dFlag();
int set_iFlag();
int set_zFlag();
int set_cFlag();

int clr_nFlag();
int clr_vFlag();
int clr_bFlag();
int clr_dFlag();
int clr_iFlag();
int clr_zFlag();
int clr_cFlag();

#endif
