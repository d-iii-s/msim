static exc_t instr_dsub(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rs = cpu->regs[instr.r.rs].val;
		uint64_t rt = cpu->regs[instr.r.rt].val;
		uint64_t dif = rs - rt;
		
		if (!((rs ^ rt) & SBIT64) && ((rs ^ dif) & SBIT64))
			return excOv;
		
		cpu->regs[instr.r.rd].val = dif;
	} else
		return excRI;
	
	return excNone;
}
