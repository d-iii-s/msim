static exc_t instr_sh(r4k_cpu_t *cpu, instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	return cpu_write_mem16(cpu, addr, (uint16_t) cpu->regs[instr.i.rt].lo,
	    true);
}

static void mnemonics_sh(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sh");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
