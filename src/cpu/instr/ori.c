static exc_t instr_ori(cpu_t *cpu, instr_t instr)
{
	uint64_t rs = cpu->regs[instr.i.rs].val;
	uint64_t imm = instr.i.imm;
	
	cpu->regs[instr.i.rt].val = rs | imm;
	return excNone;
}

static void mnemonics_ori(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "ori");
	disassemble_rt_rs_uimm(instr, mnemonics, comments);
}
