static exc_t instr_bc0f(cpu_t *cpu, instr_t instr)
{
	if (CP0_USABLE(cpu))
		/* Ignore (always false) */
		return excNone;
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}
