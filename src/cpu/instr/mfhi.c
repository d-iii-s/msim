static exc_t instr_mfhi(cpu_t *cpu, instr_t instr)
{
	cpu->regs[instr.r.rd].val = cpu->hireg.val;
	return excNone;
}
