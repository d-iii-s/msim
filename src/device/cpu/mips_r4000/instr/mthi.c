static r4k_exc_t instr_mthi(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	cpu->hireg.val = cpu->regs[instr.r.rs].val;
	return r4k_excNone;
}

static void mnemonics_mthi(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mthi");
	disassemble_rs(instr, mnemonics, comments);
}
