static exc_t instr_mul(cpu_t *cpu, instr_t instr)
{
	ASSERT(false);
	return excNone;
	/* utmp64 = ((uint64_t) urrs.lo) * ((uint64_t) urrt.lo);
	cpu->regs[ii.rd].lo = utmp64 & UINT32_C(0xffffffff); */
}
