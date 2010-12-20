static exc_t instr_bc0fl(cpu_t *cpu, instr_t instr)
{
	if (CP0_USABLE(cpu))
		/* Ignore (always false) */
		return excLikely;
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}
