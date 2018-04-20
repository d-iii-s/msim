static exc_t instr_jalr(r4k_cpu_t *cpu, instr_t instr)
{
	cpu->regs[31].val = cpu->pc.ptr + 8;
	cpu->pc_next.ptr = cpu->regs[instr.r.rs].val;
	cpu->branch = BRANCH_COND;
	return excJump;
}

static void mnemonics_jalr(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "jalr");
	if (instr.r.rd == 31)
		disassemble_rs(instr, mnemonics, comments);
	else
		disassemble_rd_rs(instr, mnemonics, comments);
}
