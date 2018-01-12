static exc_t instr_clz(cpu_t *cpu, instr_t instr)
{
	ASSERT(false);
	return excNone;
	/* utmp32 = 0;
	utmp32b = urrs.lo;
	
	while ((!(utmp32b & SBIT32)) && (utmp32 < 32)) {
		utmp32++;
		utmp32b <<= 1;
	}
	
	cpu->regs[ii.rd].lo = utmp32; */
}
