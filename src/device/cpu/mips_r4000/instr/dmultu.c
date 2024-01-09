static r4k_exc_t instr_dmultu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (CPU_64BIT_INSTRUCTION(cpu)) {
        ASSERT(false);
        return r4k_excNone;
    }

    return r4k_excRI;
}

static void mnemonics_dmultu(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "dmultu");
    disassemble_rs_rt(instr, mnemonics, comments);
}
