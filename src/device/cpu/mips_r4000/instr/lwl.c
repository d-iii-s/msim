static exc_t instr_lwl(cpu_t *cpu, instr_t instr)
{
	ptr64_t base;
	base.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	ptr64_t addr;
	addr.ptr = base.ptr & ((uint64_t) ~UINT64_C(0x03));
	
	uint32_t val;
	exc_t res = cpu_read_mem32(cpu, addr, &val, true);
	
	if (res == excNone) {
		unsigned int index = base.ptr & 0x03U;
		uint32_t comb = cpu->regs[instr.i.rt].lo & shift_tab_left[index].mask;
		comb |= val << shift_tab_left[index].shift;
		cpu->regs[instr.i.rt].val = sign_extend_32_64(comb);
	}
	
	return res;
}

static void mnemonics_lwl(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lwl");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
