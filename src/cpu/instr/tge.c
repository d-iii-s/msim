static exc_t instr_tge(cpu_t *cpu, instr_t instr)
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
