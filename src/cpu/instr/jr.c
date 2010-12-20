static exc_t instr_jr(cpu_t *cpu, instr_t instr)
{
	cpu->pc_next.ptr = cpu->regs[instr.r.rs];
	cpu->branch = BRANCH_COND;
	return excJump;
}
