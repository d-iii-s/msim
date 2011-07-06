static exc_t instr_sub(cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.r.rs].lo;
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	uint32_t dif = rs - rt;
	
	if (!((rs ^ rt) & SBIT32) && ((rs ^ dif) & SBIT32))
		return excOv;
	
	cpu->regs[instr.r.rd].val = sign_extend_32_64(dif);
	
	return excNone;
}

static void mnemonics_sub(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sub");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
