static exc_t instr_tgei(cpu_t *cpu, instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (((int64_t) cpu->regs[instr.i.rs].val) >=
		    ((int64_t) sign_extend_16_64(instr.i.imm)));
	else
		cond = (((int32_t) cpu->regs[instr.i.rs].lo) >=
		    ((int32_t) sign_extend_16_32(instr.i.imm)));
	
	if (cond)
		return excTr;
	
	return excNone;
}
