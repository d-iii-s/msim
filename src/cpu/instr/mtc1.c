static exc_t instr_mtc1(cpu_t *cpu, instr_t instr)
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

static void mnemonics_mtc1(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mtc1");
	disassemble_rt_fs(instr, mnemonics, comments);
}
