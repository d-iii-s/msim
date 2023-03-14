static r4k_exc_t instr_tlbp(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	return TLBP(cpu);
}

static void mnemonics_tlbp(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tlbp");
}
