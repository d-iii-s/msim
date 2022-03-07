static exc_t instr_sltu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_MODE(cpu)) {
		uint64_t rs = cpu->regs[instr.r.rs].val;
		uint64_t rt = cpu->regs[instr.r.rt].val;
		
		cpu->regs[instr.r.rd].val = rs < rt;
	} else {
		uint32_t rs = cpu->regs[instr.r.rs].lo;
		uint32_t rt = cpu->regs[instr.r.rt].lo;
		
		cpu->regs[instr.r.rd].val = rs < rt;
	}
	
	return excNone;
}

static void mnemonics_sltu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sltu");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
