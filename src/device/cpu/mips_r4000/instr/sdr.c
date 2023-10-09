static r4k_exc_t instr_sdr(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    ASSERT(false);
    return r4k_excNone;
}

static void mnemonics_sdr(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "sdr");
    disassemble_rt_offset_base(instr, mnemonics, comments);
}
