static r4k_exc_t instr_sync(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	/* No synchronisation is needed */
	return r4k_excNone;
}

static void mnemonics_sync(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sync");
}
