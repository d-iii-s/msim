static r4k_exc_t instr_tlbwi(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	return TLBW(cpu, false);
}

static void mnemonics_tlbwi(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tlbwi");
}
