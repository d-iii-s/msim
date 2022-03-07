static exc_t instr_bc2f(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (cp0_status_cu2(cpu))
		/* Ignore (always false) */
		return excNone;
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	cp0_cause(cpu).val |= cp0_cause_ce_cu2;
	return excCpU;
}

static void mnemonics_bc2f(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "bc2f");
	disassemble_offset(addr, instr, mnemonics, comments);
}
