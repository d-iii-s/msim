static exc_t instr_sltiu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_MODE(cpu)) {
		uint64_t rs = cpu->regs[instr.i.rs].val;
		uint64_t imm = sign_extend_16_64(instr.i.imm);
		
		cpu->regs[instr.i.rt].val = rs < imm;
	} else {
		uint32_t rs = cpu->regs[instr.i.rs].lo;
		uint32_t imm = sign_extend_16_32(instr.i.imm);
		
		cpu->regs[instr.i.rt].val = rs < imm;
	}
	
	return excNone;
}

static void mnemonics_sltiu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sltiu");
	disassemble_rt_rs_imm(instr, mnemonics, comments);
}
