static exc_t instr_dadd(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rs = cpu->regs[instr.r.rs].val;
		uint64_t rt = cpu->regs[instr.r.rt].val;
		uint64_t sum = rs + rt;
		
		if (!((rs ^ rt) & SBIT64) && ((rs ^ sum) & SBIT64))
			return excOv;
		
		cpu->regs[instr.r.rd].val = sum;
	} else
		return excRI;
	
	return excNone;
}

static void mnemonics_dadd(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "dadd");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
