static exc_t instr_tlbwi(r4k_cpu_t *cpu, instr_t instr)
{
	return TLBW(cpu, false);
}

static void mnemonics_tlbwi(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tlbwi");
}
