static exc_t instr_mfc0(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CP0_USABLE(cpu)) {
		cpu->regs[instr.r.rt].val = sign_extend_32_64(cpu->cp0[instr.r.rd].lo);
		return excNone;
	}
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}

static void mnemonics_mfc0(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mfc0");
	disassemble_rt_cp0(instr, mnemonics, comments);
}
