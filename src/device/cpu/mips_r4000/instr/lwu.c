static r4k_exc_t instr_lwu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		ptr64_t addr;
		addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
		
		uint32_t val;
		r4k_exc_t res = r4k_read_mem32(cpu, addr, &val, true);
		if (res == r4k_excNone)
			cpu->regs[instr.i.rt].val = val;
		
		return res;
	}
	
	return r4k_excRI;
}

static void mnemonics_lwu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lwu");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
