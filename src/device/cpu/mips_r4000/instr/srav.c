static exc_t instr_srav(cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.r.rs].lo;
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	cpu->regs[instr.r.rd].val =
	    sign_extend_32_64((uint32_t) (((int32_t) rt) >> (rs & UINT32_C(0x001f))));
	
	return excNone;
}

static void mnemonics_srav(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "srav");
	disassemble_rd_rt_rs(instr, mnemonics, comments);
}
