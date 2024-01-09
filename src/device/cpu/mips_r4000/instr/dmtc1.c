static r4k_exc_t instr_dmtc1(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (cp0_status_cu1(cpu)) {
        /* Ignored */
        return r4k_excNone;
    }

    /* Coprocessor unusable */
    cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
    cp0_cause(cpu).val |= cp0_cause_ce_cu1;
    return r4k_excCpU;
}

static void mnemonics_dmtc1(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "dmtc1");
    disassemble_rt_fs(instr, mnemonics, comments);
}
