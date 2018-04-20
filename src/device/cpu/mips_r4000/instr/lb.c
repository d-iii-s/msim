static exc_t instr_lb(r4k_cpu_t *cpu, instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	uint8_t val;
	exc_t res = cpu_read_mem8(cpu, addr, &val, true);
	if (res == excNone)
		cpu->regs[instr.i.rt].val = sign_extend_8_64(val);
	
	return res;
}

static void mnemonics_lb(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lb");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
