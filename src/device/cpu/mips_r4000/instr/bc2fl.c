static r4k_exc_t instr_bc2fl(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (cp0_status_cu2(cpu)) {
		/* Ignore (always false) */
		cpu->pc_next.ptr += 4;
		return r4k_excNone;
	}
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	cp0_cause(cpu).val |= cp0_cause_ce_cu2;
	return r4k_excCpU;
}

static void mnemonics_bc2fl(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "bc2fl");
	disassemble_offset(addr, instr, mnemonics, comments);
}
