static r4k_exc_t instr__reserved(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    return r4k_excRI;
}

static void mnemonics__reserved(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "(reserved)");
}
