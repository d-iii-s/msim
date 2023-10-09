static r4k_exc_t instr_bc0fl(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (CP0_USABLE(cpu)) {
        /* Ignore (always false) */
        cpu->pc_next.ptr += 4;
        return r4k_excNone;
    }

    /* Coprocessor unusable */
    cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
    return r4k_excCpU;
}

static void mnemonics_bc0fl(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "bc0fl");
    disassemble_offset(addr, instr, mnemonics, comments);
}
