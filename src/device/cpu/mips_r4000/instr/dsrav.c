static r4k_exc_t instr_dsrav(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rs = cpu->regs[instr.r.rs].val;
		uint64_t rt = cpu->regs[instr.r.rt].val;
		
		cpu->regs[instr.r.rd].val =
		    (uint64_t) (((int64_t) rt) >> (rs & UINT64_C(0x003f)));
	} else
		return r4k_excRI;
	
	return r4k_excNone;
}

static void mnemonics_dsrav(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "dsrav");
	disassemble_rd_rt_rs(instr, mnemonics, comments);
}
