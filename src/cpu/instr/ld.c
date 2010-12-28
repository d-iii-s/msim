static exc_t instr_ld(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		ptr64_t addr;
		addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
		
		uint64_t val;
		exc_t res = cpu_read_mem64(cpu, addr, &val, true);
		if (res == excNone)
			cpu->regs[instr.i.rt].val = val;
		
		return res;
	}
	
	return excRI;
}
