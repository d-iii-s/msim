static exc_t instr_lbu(cpu_t *cpu, instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	uint8_t val;
	exc_t res = cpu_read_mem8(cpu, addr, &val, true);
	if (res == excNone)
		cpu->regs[instr.i.rt].val = val;
	
	return res;
}
