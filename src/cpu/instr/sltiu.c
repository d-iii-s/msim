static exc_t instr_sltiu(cpu_t *cpu, instr_t instr)
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
