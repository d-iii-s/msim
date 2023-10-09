static r4k_exc_t instr__xcrd(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (!machine_specific_instructions) {
        return instr__reserved(cpu, instr);
    }

    alert("XCRD: CP0 register dump");
    r4k_cp0_dump_all(cpu);
    return r4k_excNone;
}

static void mnemonics__xcrd(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    if (!machine_specific_instructions) {
        return mnemonics__reserved(addr, instr, mnemonics, comments);
    }

    string_printf(mnemonics, "_xcrd");
}
