static exc_t instr_addu(r4k_cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.r.rs].lo;
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	cpu->regs[instr.r.rd].val = sign_extend_32_64(rs + rt);
	return excNone;
}

static void mnemonics_addu(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "addu");
	disassemble_rd_rs_rt(instr, mnemonics, comments);
}
