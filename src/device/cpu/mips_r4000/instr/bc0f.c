static exc_t instr_bc0f(r4k_cpu_t *cpu, instr_t instr)
{
	if (CP0_USABLE(cpu))
		/* Ignore (always false) */
		return excNone;
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}

static void mnemonics_bc0f(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "bc0f");
	disassemble_offset(addr, instr, mnemonics, comments);
}
