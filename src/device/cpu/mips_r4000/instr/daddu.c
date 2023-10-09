static r4k_exc_t instr_daddu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rs = cpu->regs[instr.r.rs].val;
		uint64_t rt = cpu->regs[instr.r.rt].val;

		cpu->regs[instr.r.rd].val = rs + rt;
	} else
		return r4k_excRI;

	return r4k_excNone;
}

static void mnemonics_daddu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "daddu");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
