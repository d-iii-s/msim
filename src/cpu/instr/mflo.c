static exc_t instr_mflo(cpu_t *cpu, instr_t instr)
{
	cpu->regs[instr.r.rd].val = cpu->loreg.val;
	return excNone;
}
