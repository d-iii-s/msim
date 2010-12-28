static exc_t instr_multu(cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.r.rs].lo;
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	/* Quick test */
	if ((rs == 0) || (rt == 0)) {
		cpu->loreg.val = 0;
		cpu->hireg.val = 0;
		return excNone;
	}
	
	uint64_t res = ((uint64_t) rs) * ((uint64_t) rt);
	cpu->loreg.val = sign_extend_32_64((uint32_t) res);
	cpu->hireg.val = sign_extend_32_64((uint32_t) (res >> 32));
	
	return excNone;
}
