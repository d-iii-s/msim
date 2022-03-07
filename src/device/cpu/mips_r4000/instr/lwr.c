static exc_t instr_lwr(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	ptr64_t base;
	base.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	ptr64_t addr;
	addr.ptr = base.ptr & ((uint64_t) ~UINT64_C(0x03));
	
	uint32_t val;
	exc_t res = cpu_read_mem32(cpu, addr, &val, true);
	
	if (res == excNone) {
		unsigned int index = base.ptr & 0x03U;
		uint32_t comb = cpu->regs[instr.i.rt].lo & shift_tab_right[index].mask;
		comb |= (val >> shift_tab_right[index].shift)
		    & (~shift_tab_right[index].mask);
		
		if (index == 0)
			cpu->regs[instr.i.rt].val = sign_extend_32_64(comb);
		else
			cpu->regs[instr.i.rt].val = comb;
	}
	
	return res;
}

static void mnemonics_lwr(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lwr");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
