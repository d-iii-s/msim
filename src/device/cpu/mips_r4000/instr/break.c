static r4k_exc_t instr_break(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    return r4k_excBp;
}

static void mnemonics_break(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "break");
}
