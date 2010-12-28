static exc_t instr_sh(cpu_t *cpu, instr_t instr)
{
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	return cpu_write_mem16(cpu, addr, (uint16_t) cpu->regs[instr.i.rt].lo,
	    true);
}
