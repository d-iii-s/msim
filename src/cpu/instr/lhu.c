static exc_t instr_lhu(cpu_t *cpu, instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	uint16_t val;
	exc_t res = cpu_read_mem16(cpu, addr, &val, true);
	if (res == excNone)
		cpu->regs[instr.i.rt].val = val;
	
	return res;
}
