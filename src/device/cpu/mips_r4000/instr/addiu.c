static exc_t instr_addiu(r4k_cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.i.rs].lo;
	uint32_t imm = sign_extend_16_32(instr.i.imm);
	
	cpu->regs[instr.i.rt].val = sign_extend_32_64(rs + imm);
	return excNone;
}

static void mnemonics_addiu(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	if (instr.i.rs == 0) {
		string_printf(mnemonics, "li");
		disassemble_rt_imm(instr, mnemonics, comments);
	} else {
		string_printf(mnemonics, "addiu");
		disassemble_rt_rs_imm(instr, mnemonics, comments);
	}
}
