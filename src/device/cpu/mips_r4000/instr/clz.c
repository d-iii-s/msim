static r4k_exc_t instr_clz(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	ASSERT(false);
	return r4k_excNone;
	/* utmp32 = 0;
	utmp32b = urrs.lo;
	
	while ((!(utmp32b & SBIT32)) && (utmp32 < 32)) {
		utmp32++;
		utmp32b <<= 1;
	}
	
	cpu->regs[ii.rd].lo = utmp32; */
}
