static exc_t instr_teqi(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (cpu->regs[instr.i.rs].val == sign_extend_16_64(instr.i.imm));
	else
		cond = (cpu->regs[instr.i.rs].lo == sign_extend_16_32(instr.i.imm));
	
	if (cond)
		return excTr;
	
	return excNone;
}

static void mnemonics_teqi(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "teqi");
	disassemble_rs_imm(instr, mnemonics, comments);
}
