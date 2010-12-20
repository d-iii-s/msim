static exc_t instr_addiu(cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.i.rs].lo;
	uint32_t imm = sign_extend_16_32(instr.i.imm);
	
	cpu->regs[instr.i.rt].val = sign_extend_32_64(rs + imm);
	return excNone;
}
