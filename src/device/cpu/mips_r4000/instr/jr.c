static exc_t instr_jr(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	cpu->pc_next.ptr = cpu->regs[instr.r.rs].val;
	cpu->branch = BRANCH_COND;
	return excJump;
}

static void mnemonics_jr(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "jr");
	disassemble_rs(instr, mnemonics, comments);
}
