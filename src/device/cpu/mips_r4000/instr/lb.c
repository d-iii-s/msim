static r4k_exc_t instr_lb(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);

	uint8_t val;
	r4k_exc_t res = cpu_read_mem8(cpu, addr, &val, true);
	if (res == r4k_excNone)
		cpu->regs[instr.i.rt].val = sign_extend_8_64(val);

	return res;
}

static void mnemonics_lb(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lb");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
