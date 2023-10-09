static r4k_exc_t instr_tlbr(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    return TLBR(cpu);
}

static void mnemonics_tlbr(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "tlbr");
}
