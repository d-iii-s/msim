static exc_t instr_lwc1(cpu_t *cpu, instr_t instr)
{
	if (cp0_status_cu1(cpu)) {
		/* Ignored */
		return excNone;
	}
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	cp0_cause(cpu).val |= cp0_cause_ce_cu1;
	return excCpU;
}

static void mnemonics_lwc1(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lwc1");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
