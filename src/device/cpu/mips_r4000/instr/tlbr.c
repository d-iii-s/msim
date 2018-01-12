static exc_t instr_tlbr(cpu_t *cpu, instr_t instr)
{
	return TLBR(cpu);
}

static void mnemonics_tlbr(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tlbr");
}
