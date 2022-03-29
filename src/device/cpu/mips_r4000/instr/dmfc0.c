static r4k_exc_t instr_dmfc0(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		if (CP0_USABLE(cpu)) {
			cpu->regs[instr.r.rt].val = cpu->cp0[instr.r.rd].val;
			return r4k_excNone;
		}
		
		/* Coprocessor unusable */
		cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
		return r4k_excCpU;
	}
	
	return r4k_excRI;
}

static void mnemonics_dmfc0(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "dmfc0");
	disassemble_rt_cp0(instr, mnemonics, comments);
}
