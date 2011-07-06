static exc_t instr_dmfc1(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		if (cp0_status_cu1(cpu)) {
			/* Ignored */
			return excNone;
		}
		
		/* Coprocessor unusable */
		cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		cp0_cause(cpu).val |= cp0_cause_ce_cu1;
		return excCpU;
	}
	
	return excRI;
}

static void mnemonics_dmfc1(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "dmfc1");
	disassemble_rt_fs(instr, mnemonics, comments);
}
