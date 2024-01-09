static r4k_exc_t instr_bc0f(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (CP0_USABLE(cpu)) {
        /* Ignore (always false) */
        return r4k_excNone;
    }

    /* Coprocessor unusable */
    cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
    return r4k_excCpU;
}

static void mnemonics_bc0f(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "bc0f");
    disassemble_offset(addr, instr, mnemonics, comments);
}
