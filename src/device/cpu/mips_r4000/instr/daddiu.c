static exc_t instr_daddiu(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rs = cpu->regs[instr.i.rs].val;
		uint64_t imm = sign_extend_16_64(instr.i.imm);
		
		cpu->regs[instr.i.rt].val = rs + imm;
	} else
		return excRI;
	
	return excNone;
}

static void mnemonics_daddiu(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "daddiu");
	disassemble_rt_rs_imm(instr, mnemonics, comments);
}
