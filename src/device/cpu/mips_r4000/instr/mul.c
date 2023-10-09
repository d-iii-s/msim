static r4k_exc_t instr_mul(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    ASSERT(false);
    return r4k_excNone;
    /* utmp64 = ((uint64_t) urrs.lo) * ((uint64_t) urrt.lo);
    cpu->regs[ii.rd].lo = utmp64 & UINT32_C(0xffffffff); */
}
