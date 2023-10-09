static r4k_exc_t instr_tltu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	bool cond;

	if (CPU_64BIT_MODE(cpu))
		cond = (cpu->regs[instr.r.rs].val < cpu->regs[instr.r.rt].val);
	else
		cond = (cpu->regs[instr.r.rs].lo < cpu->regs[instr.r.rt].lo);

	if (cond)
		return r4k_excTr;

	return r4k_excNone;
}

static void mnemonics_tltu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tltu");
	disassemble_rs_rt(instr, mnemonics, comments);
}
