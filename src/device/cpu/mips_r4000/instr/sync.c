static exc_t instr_sync(r4k_cpu_t *cpu, instr_t instr)
{
	/* No synchronisation is needed */
	return excNone;
}

static void mnemonics_sync(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sync");
}
