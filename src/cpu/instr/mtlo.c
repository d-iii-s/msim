static exc_t instr_mtlo(cpu_t *cpu, instr_t instr)
{
	cpu->loreg.val = cpu->regs[instr.r.rs].val;
	return excNone;
}
