static exc_t instr_dmult(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		ASSERT(false);
		return excNone;
	}
	
	return excRI;
}
