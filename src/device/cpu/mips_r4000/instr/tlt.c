static exc_t instr_tlt(r4k_cpu_t *cpu, instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (((int64_t) cpu->regs[instr.r.rs].val) <
		    ((int64_t) cpu->regs[instr.r.rt].val));
	else
		cond = (((int32_t) cpu->regs[instr.r.rs].lo) <
		    ((int32_t) cpu->regs[instr.r.rt].lo));
	
	if (cond)
		return excTr;
	
	return excNone;
}

static void mnemonics_tlt(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tlt");
	disassemble_rs_rt(instr, mnemonics, comments);
}
