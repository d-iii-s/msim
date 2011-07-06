static exc_t instr_nor(cpu_t *cpu, instr_t instr)
{
	uint64_t rs = cpu->regs[instr.r.rs].val;
	uint64_t rt = cpu->regs[instr.r.rt].val;
	
	cpu->regs[instr.r.rd].val = ~(rs | rt);
	return excNone;
}

static void mnemonics_nor(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "nor");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
