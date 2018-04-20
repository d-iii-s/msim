static exc_t instr_ld(r4k_cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		ptr64_t addr;
		addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
		
		uint64_t val;
		exc_t res = cpu_read_mem64(cpu, addr, &val, true);
		if (res == excNone)
			cpu->regs[instr.i.rt].val = val;
		
		return res;
	}
	
	return excRI;
}

static void mnemonics_ld(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "ld");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
