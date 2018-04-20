static exc_t instr_mtlo(r4k_cpu_t *cpu, instr_t instr)
{
	cpu->loreg.val = cpu->regs[instr.r.rs].val;
	return excNone;
}

static void mnemonics_mtlo(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mtlo");
	disassemble_rs(instr, mnemonics, comments);
}
