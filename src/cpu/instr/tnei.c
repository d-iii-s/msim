static exc_t instr_tnei(cpu_t *cpu, instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (cpu->regs[instr.i.rs].val != sign_extend_16_64(instr.i.imm));
	else
		cond = (cpu->regs[instr.i.rs].lo != sign_extend_16_32(instr.i.imm));
	
	if (cond)
		return excTr;
	
	return excNone;
}