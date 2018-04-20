static exc_t instr_j(r4k_cpu_t *cpu, instr_t instr)
{
	cpu->pc_next.ptr =
	    (cpu->pc_next.ptr & TARGET_COMB) | (instr.j.target << TARGET_SHIFT);
	cpu->branch = BRANCH_COND;
	return excJump;
}

static void mnemonics_j(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "j");
	disassemble_target(addr, instr, mnemonics, comments);
}
