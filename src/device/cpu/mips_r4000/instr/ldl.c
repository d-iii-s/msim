static r4k_exc_t instr_ldl(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    ASSERT(false);
    return r4k_excNone;
}

static void mnemonics_ldl(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "ldl");
    disassemble_rt_offset_base(instr, mnemonics, comments);
}
