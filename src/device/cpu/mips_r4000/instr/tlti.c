static exc_t instr_tlti(r4k_cpu_t *cpu, instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (((int64_t) cpu->regs[instr.r.rs].val) <
		    ((int64_t) sign_extend_16_64(instr.i.imm)));
	else
		cond = (((int32_t) cpu->regs[instr.r.rs].lo) <
		    ((int32_t) sign_extend_16_32(instr.i.imm)));
	
	if (cond)
		return excTr;
	
	return excNone;
}

static void mnemonics_tlti(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tlti");
	disassemble_rs_imm(instr, mnemonics, comments);
}
