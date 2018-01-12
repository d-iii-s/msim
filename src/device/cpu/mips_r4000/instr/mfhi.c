static exc_t instr_mfhi(cpu_t *cpu, instr_t instr)
{
	cpu->regs[instr.r.rd].val = cpu->hireg.val;
	return excNone;
}

static void mnemonics_mfhi(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mfhi");
	disassemble_rd(instr, mnemonics, comments);
}
