static exc_t instr_madd(cpu_t *cpu, instr_t instr)
{
	ASSERT(false);
	return excNone;
	/* utmp64 = ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
	multiply(cpu, urrs.lo, urrt.lo, true);
	utmp64 -= ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
	cpu->hireg.lo = utmp64 >> 32;
	cpu->loreg.lo = utmp64 & UINT32_C(0xffffffff); */
}
