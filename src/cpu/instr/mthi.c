static exc_t instr_mthi(cpu_t *cpu, instr_t instr)
{
	cpu->hireg.val = cpu->regs[instr.r.rs].val;
	return excNone;
}
