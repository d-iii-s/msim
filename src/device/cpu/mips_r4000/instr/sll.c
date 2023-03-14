static r4k_exc_t instr_sll(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	cpu->regs[instr.r.rd].val = sign_extend_32_64(rt << instr.r.sa);
	
	return r4k_excNone;
}

static void mnemonics_sll(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	if ((instr.r.rd == 0) && (instr.r.rt == 0) && (instr.r.sa == 0))
		string_printf(mnemonics, "nop");
	else {
		string_printf(mnemonics, "sll");
		disassemble_rd_rt_sa(instr, mnemonics, comments);
	}
}
