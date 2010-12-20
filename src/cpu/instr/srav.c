static exc_t instr_srav(cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.r.rs].lo;
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	cpu->regs[instr.r.rd].val =
	    sign_extend_32_64((uint32_t) (((int32_t) rt) >> (rs & UINT32_C(0x001f))));
	
	return excNone;
}
