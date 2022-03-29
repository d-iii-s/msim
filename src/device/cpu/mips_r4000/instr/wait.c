static r4k_exc_t instr_wait(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	ASSERT(false);
	cpu->pc_next.ptr = cpu->pc.ptr;
	cpu->stdby = true;
	return r4k_excNone;
}
