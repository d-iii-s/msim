static exc_t instr_sdl(r4k_cpu_t *cpu, instr_t instr)
{
	ASSERT(false);
	return excNone;
}

static void mnemonics_sdl(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sdl");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
