static exc_t instr_ldr(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	ASSERT(false);
	return excNone;
}

static void mnemonics_ldr(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "ldr");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
