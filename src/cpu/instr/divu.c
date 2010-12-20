static exc_t instr_divu(cpu_t *cpu, instr_t instr)
{
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	if (rt == 0) {
		cpu->loreg.val = 0;
		cpu->hireg.val = 0;
	} else {
		uint32_t rs = cpu->regs[instr.r.rs].lo;
		
		cpu->loreg.val = sign_extend_32_64(rs / rt);
		cpu->hireg.val = sign_extend_32_64(rs % rt);
	}
	
	return excNone;
}
