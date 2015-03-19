#include "instructions.h"
#include "cpu.h"
#include "cpu_memory.h"

int asm_adc(uint16_t addr) {
	uint16_t op1 = get_a();
	uint16_t op2 = mem_read_cpu(addr);
	uint16_t carry = get_cFlag();
	uint16_t result = op1 + op2 + carry;

	set_a((uint8_t) result);

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (((0x80 & op1) == (0x80 & op2)) && ((0x80 & op1) != (0x80 & result)))
		set_vFlag();
	else
		clr_vFlag();

	if (!get_a())
		set_zFlag();
	else 
		clr_zFlag();

	if (0xff00 & result)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_and(uint16_t addr) {
	set_a(mem_read_cpu(addr) & get_a());

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_asl(uint16_t addr) {
	uint8_t value = mem_read_cpu(addr);
	mem_write_cpu(addr, value << 1);

	if (0x80 & mem_read_cpu(addr))
		set_nFlag();
	else
		clr_nFlag();

	if (!mem_read_cpu(addr))
		set_zFlag();
	else
		clr_zFlag();

	if (0x80 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_asla(uint16_t addr) {
	uint8_t value = get_a();;
	set_a(value << 1);

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	if (0x80 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_bit(uint16_t addr) {
	uint8_t op1 = get_a();
	uint8_t op2 = mem_read_cpu(addr);

	if (0x80 & op2)
		set_nFlag();
	else
		clr_nFlag();

	if (0x40 & op2)
		set_vFlag();
	else
		clr_vFlag();

	if (!(op1 & op2))
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_bpl(uint16_t addr) {
	if (!get_nFlag())
		set_pc(addr);

	return 0;
}

int asm_bmi(uint16_t addr) {
	if (get_nFlag())
		set_pc(addr);

	return 0;
}

int asm_bvc(uint16_t addr) {
	if (!get_vFlag())
		set_pc(addr);

	return 0;
}

int asm_bvs(uint16_t addr) {
	if (get_vFlag())
		set_pc(addr);

	return 0;
}

int asm_bcc(uint16_t addr) {
	if (!get_cFlag())
		set_pc(addr);

	return 0;
}

int asm_bcs(uint16_t addr) {
	if (get_cFlag())
		set_pc(addr);

	return 0;
}

int asm_bne(uint16_t addr) {
	if (!get_zFlag())
		set_pc(addr);

	return 0;
}

int asm_beq(uint16_t addr) {
	if (get_zFlag())
		set_pc(addr);

	return 0;
}

int asm_brk(uint16_t addr) {
	uint16_t pc = get_pc() + 1;
	uint8_t pcHigh = pc >> 8;
	uint8_t pcLow = pc;

	if (!get_iFlag()) {
		// Push PC
		mem_write_cpu((0x100 | (uint16_t) get_sp()), pcHigh);
		set_sp(get_sp()-1);
		mem_write_cpu((0x100 | (uint16_t) get_sp()), pcLow);
		set_sp(get_sp()-1);
		// Push status
		mem_write_cpu((0x100 | (uint16_t) get_sp()), get_status());
		set_sp(get_sp()-1);

		set_iFlag();

		cpu_interrupt(IRQ);
	}

	return 0;
}

int asm_cmp(uint16_t addr) {
	uint8_t op1 = get_a();
	uint8_t op2 = mem_read_cpu(addr);
	uint8_t result = op1 - op2;

	if (0x80 & result)
		set_nFlag();
	else
		clr_nFlag();

	if (op2 <= op1)
		set_cFlag();
	else
		clr_cFlag();

	if (!result)
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_cpx(uint16_t addr) {
	uint8_t op1 = get_x();
	uint8_t op2 = mem_read_cpu(addr);
	uint8_t result = op1 - op2;

	if (0x80 & result)
		set_nFlag();
	else
		clr_nFlag();

	if (op2 <= op1)
		set_cFlag();
	else
		clr_cFlag();

	if (!result)
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_cpy(uint16_t addr) {
	uint8_t op1 = get_y();
	uint8_t op2 = mem_read_cpu(addr);
	uint8_t result = op1 - op2;

	if (0x80 & result)
		set_nFlag();
	else
		clr_nFlag();

	if (op2 <= op1)
		set_cFlag();
	else
		clr_cFlag();

	if (!result)
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_dec(uint16_t addr) {
	mem_write_cpu(addr, mem_read_cpu(addr) - 1);

	if (0x80 & mem_read_cpu(addr))
		set_nFlag();
	else
		clr_nFlag();

	if (!mem_read_cpu(addr))
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_eor(uint16_t addr) {
	set_a(mem_read_cpu(addr) ^ get_a());

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_clc(uint16_t addr) {
	clr_cFlag();

	return 0;
}

int asm_sec(uint16_t addr) {
	set_cFlag();

	return 0;
}

int asm_cli(uint16_t addr) {
	clr_iFlag();

	return 0;
}

int asm_sei(uint16_t addr) {
	set_iFlag();

	return 0;
}

int asm_clv(uint16_t addr) {
	clr_vFlag();

	return 0;
}

int asm_cld(uint16_t addr) {
	clr_dFlag();

	return 0;
}

int asm_sed(uint16_t addr) {
	set_dFlag();

	return 0;
}

int asm_inc(uint16_t addr) {
	mem_write_cpu(addr, mem_read_cpu(addr) + 1);

	if (0x80 & mem_read_cpu(addr))
		set_nFlag();
	else
		clr_nFlag();

	if (!mem_read_cpu(addr))
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_jmp(uint16_t addr) {
	set_pc(addr);

	return 0;
}

int asm_jsr(uint16_t addr) {
	uint16_t pc = get_pc() - 1;
	uint8_t pcHigh = pc >> 8;
	uint8_t pcLow = pc;

	mem_write_cpu((0x100 | (uint16_t) get_sp()), pcHigh);
	set_sp(get_sp()-1);
	mem_write_cpu((0x100 | (uint16_t) get_sp()), pcLow);
	set_sp(get_sp()-1);

	set_pc(addr);

	return 0;
}

int asm_lda(uint16_t addr) {
	set_a(mem_read_cpu(addr));

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_ldx(uint16_t addr) {
	set_x(mem_read_cpu(addr));

	if (0x80 & get_x())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_x())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_ldy(uint16_t addr) {
	set_y(mem_read_cpu(addr));

	if (0x80 & get_y())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_y())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_lsr(uint16_t addr) {
	uint8_t value = mem_read_cpu(addr);
	mem_write_cpu(addr, value >> 1);

	if (0x80 & mem_read_cpu(addr))
		set_nFlag();
	else
		clr_nFlag();

	if (!mem_read_cpu(addr))
		set_zFlag();
	else
		clr_zFlag();

	if (0x01 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_lsra(uint16_t addr) {
	uint8_t value = get_a();
	set_a(value >> 1);

	if (0x80 & get_a(addr))
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	if (0x01 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_nop(uint16_t addr) {
	return 0;
}

int asm_ora(uint16_t addr) {
	set_a(mem_read_cpu(addr) | get_a());

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_tax(uint16_t addr) {
	set_x(get_a());

	if (0x80 & get_x())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_x())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_txa(uint16_t addr) {
	set_a(get_x());

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_dex(uint16_t addr) {
	set_x(get_x() - 1);

	if (0x80 & get_x())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_x())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_inx(uint16_t addr) {
	set_x(get_x() + 1);

	if (0x80 & get_x())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_x())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_tay(uint16_t addr) {
	set_y(get_a());

	if (0x80 & get_y())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_y())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_tya(uint16_t addr) {
	set_a(get_y());

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_dey(uint16_t addr) {
	set_y(get_y() - 1);

	if (0x80 & get_y())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_y())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_iny(uint16_t addr) {
	set_y(get_y() + 1);

	if (0x80 & get_y())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_y())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_rol(uint16_t addr) {
	uint8_t value = mem_read_cpu(addr);
	mem_write_cpu(addr, (value << 1) | get_cFlag());

	if (0x80 & mem_read_cpu(addr))
		set_nFlag();
	else
		clr_nFlag();

	if (!mem_read_cpu(addr))
		set_zFlag();
	else
		clr_zFlag();

	if (0x80 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_rola(uint16_t addr) {
	uint8_t value = get_a();
	set_a((value << 1) | get_cFlag());

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	if (0x80 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_ror(uint16_t addr) {
	uint8_t value = mem_read_cpu(addr);
	mem_write_cpu(addr, (value >> 1) | (get_cFlag() << 7));

	if (0x80 & mem_read_cpu(addr))
		set_nFlag();
	else
		clr_nFlag();

	if (!mem_read_cpu(addr))
		set_zFlag();
	else
		clr_zFlag();

	if (0x01 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_rora(uint16_t addr) {
	uint8_t value = get_a();
	set_a((value >> 1) | (get_cFlag() << 7));

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	if (0x01 & value)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_rti(uint16_t addr) {
	uint16_t pcHigh, pcLow;

	// Pull status
	set_sp(get_sp()+1);
	set_status(mem_read_cpu(0x100 | (uint16_t) get_sp()));
	// Pull PC
	set_sp(get_sp()+1);
	pcLow = mem_read_cpu(0x100 | (uint16_t) get_sp());
	set_sp(get_sp()+1);
	pcHigh = mem_read_cpu(0x100 | (uint16_t) get_sp());
	set_pc((pcHigh << 8) | pcLow);

	return 0;
}

int asm_rts(uint16_t addr) {
	uint16_t pcHigh, pcLow;

	set_sp(get_sp()+1);
	pcLow = mem_read_cpu(0x100 | (uint16_t) get_sp());
	set_sp(get_sp()+1);
	pcHigh = mem_read_cpu(0x100 | (uint16_t) get_sp());

	set_pc((pcHigh << 8) | pcLow);
	set_pc(get_pc()+1);

	return 0;
}

int asm_sbc(uint16_t addr) {
	uint16_t op1 = get_a();
	uint16_t op2 = ~mem_read_cpu(addr) & 0x00ff;
	uint16_t carry = get_cFlag();
	uint16_t result = op1 + op2 + carry;

	set_a((uint8_t) result);

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (((0x80 & op1) == (0x80 & op2)) && ((0x80 & op1) != (0x80 & result)))
		set_vFlag();
	else
		clr_vFlag();

	if (!get_a())
		set_zFlag();
	else 
		clr_zFlag();

	if (0xff00 & result)
		set_cFlag();
	else
		clr_cFlag();

	return 0;
}

int asm_sta(uint16_t addr) {
	mem_write_cpu(addr, get_a());

	return 0;
}

int asm_txs(uint16_t addr) {
	set_sp(get_x());

	/*if (0x80 & get_sp())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_sp())
		set_zFlag();
	else
		clr_zFlag();*/

	return 0;
}

int asm_tsx(uint16_t addr) {
	set_x(get_sp());

	if (0x80 & get_x())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_x())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_pha(uint16_t addr) {
	mem_write_cpu((0x100 | (uint16_t) get_sp()), get_a());
	set_sp(get_sp()-1);

	return 0;
}

int asm_pla(uint16_t addr) {
	set_sp(get_sp()+1);
	set_a(mem_read_cpu(0x100 | (uint16_t) get_sp()));

	if (0x80 & get_a())
		set_nFlag();
	else
		clr_nFlag();

	if (!get_a())
		set_zFlag();
	else
		clr_zFlag();

	return 0;
}

int asm_php(uint16_t addr) {
	mem_write_cpu((0x100 | (uint16_t) get_sp()), 0x10 | get_status());
	set_sp(get_sp()-1);

	return 0;
}

int asm_plp(uint16_t addr) {
	set_sp(get_sp()+1);
	set_status(mem_read_cpu(0x100 | (uint16_t) get_sp()));

	return 0;
}

int asm_stx(uint16_t addr) {
	mem_write_cpu(addr, get_x());

	return 0;
}

int asm_sty(uint16_t addr) {
	mem_write_cpu(addr, get_y());

	return 0;
}

