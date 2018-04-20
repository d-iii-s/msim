static exc_t instr_mthi(r4k_cpu_t *cpu, instr_t instr)
{
	cpu->hireg.val = cpu->regs[instr.r.rs].val;
	return excNone;
}

static void mnemonics_mthi(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mthi");
	disassemble_rs(instr, mnemonics, comments);
}
