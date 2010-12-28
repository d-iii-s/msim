static exc_t instr_sd(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		ptr64_t addr;
		addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
		
		return cpu_write_mem64(cpu, addr, cpu->regs[instr.i.rt].val, true);
	}
	
	return excRI;
}
