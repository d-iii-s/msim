static exc_t instr_bc2f(cpu_t *cpu, instr_t instr)
{
	if (cp0_status_cu2(cpu))
		/* Ignore (always false) */
		return excNone;
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	cp0_cause(cpu).val |= cp0_cause_ce_cu2;
	return excCpU;
}

static void mnemonics_bc2f(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "bc2f");
	disassemble_offset(addr, instr, mnemonics, comments);
}
