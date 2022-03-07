static exc_t instr_slti(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_MODE(cpu)) {
		uint64_t rs = cpu->regs[instr.i.rs].val;
		uint64_t imm = sign_extend_16_64(instr.i.imm);
		
		cpu->regs[instr.i.rt].val = ((int64_t) rs) < ((int64_t) imm);
	} else {
		uint32_t rs = cpu->regs[instr.i.rs].lo;
		uint32_t imm = sign_extend_16_32(instr.i.imm);
		
		cpu->regs[instr.i.rt].val = ((int32_t) rs) < ((int32_t) imm);
	}
	
	return excNone;
}

static void mnemonics_slti(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "slti");
	disassemble_rt_rs_imm(instr, mnemonics, comments);
}
