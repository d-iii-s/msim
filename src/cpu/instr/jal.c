static exc_t instr_jal(cpu_t *cpu, instr_t instr)
{
	cpu->regs[31].val = cpu->pc.ptr + 8;
	cpu->pc_next.ptr =
	    (cpu->pc_next.ptr & TARGET_COMB) | (instr.j.target << TARGET_SHIFT);
	cpu->branch = BRANCH_COND;
	return excJump;
}
