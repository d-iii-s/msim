static exc_t instr_xor(cpu_t *cpu, instr_t instr)
{
	uint64_t rs = cpu->regs[instr.r.rs].val;
	uint64_t rt = cpu->regs[instr.r.rt].val;
	
	cpu->regs[instr.r.rd].val = rs ^ rt;
	return excNone;
}
