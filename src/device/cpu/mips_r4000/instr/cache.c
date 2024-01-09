static r4k_exc_t instr_cache(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    ASSERT(false);
    return r4k_excNone;
}

static void mnemonics_cache(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "cache");

    // FIXME: decode operation properly
    disassemble_rt_offset_base(instr, mnemonics, comments);
}
