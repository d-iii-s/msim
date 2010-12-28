static exc_t instr_tlbwr(cpu_t *cpu, instr_t instr)
{
	return TLBW(cpu, true);
}
