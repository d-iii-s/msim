static exc_t instr_tlbwi(cpu_t *cpu, instr_t instr)
{
	return TLBW(cpu, false);
}
