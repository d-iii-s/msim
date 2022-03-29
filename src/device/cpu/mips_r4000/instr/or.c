static r4k_exc_t instr_or(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	uint64_t rs = cpu->regs[instr.r.rs].val;
	uint64_t rt = cpu->regs[instr.r.rt].val;
	
	cpu->regs[instr.r.rd].val = rs | rt;
	return r4k_excNone;
}

static void mnemonics_or(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "or");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
