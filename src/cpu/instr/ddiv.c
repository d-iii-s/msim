static exc_t instr_ddiv(cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		uint64_t rt = cpu->regs[instr.r.rt].val;
		
		if (rt == 0) {
			cpu->loreg.val = 0;
			cpu->hireg.val = 0;
		} else {
			uint64_t rs = cpu->regs[instr.r.rs].val;
			
			cpu->loreg.val = (uint64_t) (((int64_t) rs) / ((int64_t) rt));
			cpu->hireg.val = (uint64_t) (((int64_t) rs) % ((int64_t) rt));
		}
	} else
		return excRI;
	
	return excNone;
}
