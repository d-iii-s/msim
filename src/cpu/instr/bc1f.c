static exc_t instr_bc1f(cpu_t *cpu, instr_t instr)
{
	if (cp0_status_cu1(cpu))
		/* Ignore (always false) */
		return excNone;
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	cp0_cause(cpu).val |= cp0_cause_ce_cu1;
	return excCpU;
}

static void mnemonics_bc1f(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "bc1f");
	disassemble_offset(addr, instr, mnemonics, comments);
}
