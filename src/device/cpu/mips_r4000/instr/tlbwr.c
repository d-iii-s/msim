static exc_t instr_tlbwr(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	return TLBW(cpu, true);
}

static void mnemonics_tlbwr(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "tlbwr");
}
