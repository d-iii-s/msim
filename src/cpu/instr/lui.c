static exc_t instr_lui(cpu_t *cpu, instr_t instr)
{
	cpu->regs[instr.i.rt].val =
	    sign_extend_32_64(((uint32_t) instr.i.imm) << 16);
	
	return excNone;
}
