static exc_t instr_tgeu(cpu_t *cpu, instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (cpu->regs[instr.r.rs].val >= cpu->regs[instr.r.rt].val);
	else
		cond = (cpu->regs[instr.r.rs].lo >= cpu->regs[instr.r.rt].lo);
	
	if (cond)
		return excTr;
	
	return excNone;
}
