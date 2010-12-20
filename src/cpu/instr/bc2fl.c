static exc_t instr_bc2fl(cpu_t *cpu, instr_t instr)
{
	if (cp0_status_cu2(cpu))
		/* Ignore (always false) */
		return excLikely;
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	cp0_cause(cpu).val |= cp0_cause_ce_cu2;
	return excCpU;
}
