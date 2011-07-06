static exc_t instr_ldl(cpu_t *cpu, instr_t instr)
{
	ASSERT(false);
	return excNone;
}

static void mnemonics_ldl(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "ldl");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
