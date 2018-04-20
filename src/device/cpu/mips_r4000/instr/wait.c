static exc_t instr_wait(r4k_cpu_t *cpu, instr_t instr)
{
	ASSERT(false);
	cpu->pc_next.ptr = cpu->pc.ptr;
	cpu->stdby = true;
	return excNone;
}
