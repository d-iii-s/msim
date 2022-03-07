static exc_t instr_lw(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	uint32_t val;
	exc_t res = cpu_read_mem32(cpu, addr, &val, true);
	if (res == excNone)
		cpu->regs[instr.i.rt].val = sign_extend_32_64(val);
	
	return res;
}

static void mnemonics_lw(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lw");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
