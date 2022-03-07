static exc_t instr_dsubu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rs = cpu->regs[instr.r.rs].val;
		uint64_t rt = cpu->regs[instr.r.rt].val;
		
		cpu->regs[instr.r.rd].val = rs - rt;
	} else
		return excRI;
	
	return excNone;
}

static void mnemonics_dsubu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "dsubu");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
