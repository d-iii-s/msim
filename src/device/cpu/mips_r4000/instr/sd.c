static exc_t instr_sd(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		ptr64_t addr;
		addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
		
		return cpu_write_mem64(cpu, addr, cpu->regs[instr.i.rt].val, true);
	}
	
	return excRI;
}

static void mnemonics_sd(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sd");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
