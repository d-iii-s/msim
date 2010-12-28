static exc_t instr_ldc2(cpu_t *cpu, instr_t instr)
{
	if (cp0_status_cu2(cpu)) {
		/* Ignored */
		return excNone;
	}
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	cp0_cause(cpu).val |= cp0_cause_ce_cu2;
	return excCpU;
}
