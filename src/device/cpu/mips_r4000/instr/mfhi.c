static r4k_exc_t instr_mfhi(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	cpu->regs[instr.r.rd].val = cpu->hireg.val;
	return r4k_excNone;
}

static void mnemonics_mfhi(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mfhi");
	disassemble_rd(instr, mnemonics, comments);
}
