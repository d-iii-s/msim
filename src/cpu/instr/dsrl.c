static exc_t instr_dsrl(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rt = cpu->regs[instr.r.rt].val;
		cpu->regs[instr.r.rd].val = rt >> instr.r.sa;
	} else
		return excRI;
	
	return excNone;
}
