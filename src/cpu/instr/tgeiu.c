static exc_t instr_tgeiu(cpu_t *cpu, instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (cpu->regs[instr.i.rs].val >= ((uint64_t) instr.i.imm));
	else
		cond = (cpu->regs[instr.i.rs].lo >= ((uint32_t) instr.i.imm));
	
	if (cond)
		return excTr;
	
	return excNone;
}
