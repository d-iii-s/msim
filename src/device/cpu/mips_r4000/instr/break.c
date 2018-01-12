static exc_t instr_break(cpu_t *cpu, instr_t instr)
{
	return excBp;
}

static void mnemonics_break(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "break");
}
