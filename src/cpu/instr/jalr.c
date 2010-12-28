static exc_t instr_jalr(cpu_t *cpu, instr_t instr)
{
	cpu->regs[31].val = cpu->pc.ptr + 8;
	cpu->pc_next.ptr = cpu->regs[instr.r.rs].val;
	cpu->branch = BRANCH_COND;
	return excJump;
}
