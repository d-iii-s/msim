static exc_t instr_dmfc0(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		if (CP0_USABLE(cpu)) {
			cpu->regs[instr.r.rt].val = cpu->cp0[instr.r.rd].val;
			return excNone;
		}
		
		/* Coprocessor unusable */
		cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		return excCpU;
	}
	
	return excRI;
}
