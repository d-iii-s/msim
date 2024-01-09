static r4k_exc_t instr_maddu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    ASSERT(false);
    return r4k_excNone;
    /* utmp64 = ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
    multiply(cpu, urrs.lo, urrt.lo, false);
    utmp64 += ((uint64_t) cpu->hireg.lo << 32) | cpu->loreg.lo;
    cpu->hireg.lo = utmp64 >> 32;
    cpu->loreg.lo = utmp64 & UINT32_C(0xffffffff); */
}
