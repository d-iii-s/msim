static exc_t instr_dmultu(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		ASSERT(false);
	} else
		return excRI;
	
	return excNone;
}
