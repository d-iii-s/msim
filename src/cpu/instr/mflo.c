static exc_t instr_mflo(cpu_t *cpu, instr_t instr)
{
	cpu->regs[instr.r.rd].val = cpu->loreg.val;
	return excNone;
}

static void mnemonics_mflo(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mflo");
	disassemble_rd(instr, mnemonics, comments);
}
