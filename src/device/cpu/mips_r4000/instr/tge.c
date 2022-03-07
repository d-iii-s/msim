static exc_t instr_tge(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (((int64_t) cpu->regs[instr.r.rs].val) >=
		    ((int64_t) cpu->regs[instr.r.rt].val));
	else
		cond = (((int32_t) cpu->regs[instr.r.rs].lo) >=
		    ((int32_t) cpu->regs[instr.r.rt].lo));
	
	if (cond)
		return excTr;
	
	return excNone;
}

static void mnemonics_tge(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tge");
	disassemble_rs_rt(instr, mnemonics, comments);
}
