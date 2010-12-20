static exc_t instr_j(cpu_t *cpu, instr_t instr)
{
	cpu->pc_next.ptr =
	    (cpu->pc_next.ptr & TARGET_COMB) | (instr.j.target << TARGET_SHIFT);
	cpu->branch = BRANCH_COND;
	return excJump;
}
