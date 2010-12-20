static exc_t instr_sra(cpu_t *cpu, instr_t instr)
{
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	cpu->regs[instr.r.rd].val =
	    sign_extend_32_64((uint32_t) (((int32_t) rt) >> inatr.r.sa));
	
	return excNone;
}
