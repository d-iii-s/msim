static exc_t instr_andi(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	uint64_t rs = cpu->regs[instr.i.rs].val;
	uint64_t imm = instr.i.imm;
	
	cpu->regs[instr.i.rt].val = rs & imm;
	return excNone;
}

static void mnemonics_andi(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "andi");
	disassemble_rt_rs_uimm(instr, mnemonics, comments);
}
