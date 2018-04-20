static exc_t instr_dsll(r4k_cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rt = cpu->regs[instr.r.rt].val;
		cpu->regs[instr.r.rd].val = rt << instr.r.sa;
	} else
		return excRI;
	
	return excNone;
}

static void mnemonics_dsll(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "dsll");
	disassemble_rd_rt_sa(instr, mnemonics, comments);
}
