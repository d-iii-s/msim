static exc_t instr_sw(cpu_t *cpu, instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	return cpu_write_mem32(cpu, addr, cpu->regs[instr.i.rt].lo,
	    true);
}

static void mnemonics_sw(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sw");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
