static r4k_exc_t instr__xval(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (!machine_specific_instructions) {
        return instr__reserved(cpu, instr);
    }

    alert("XVAL: Register a0 = %#" PRIx64 " = %" PRIu64 " (%" PRId64 ")",
            cpu->regs[4].val, cpu->regs[4].val, cpu->regs[4].val);
    return r4k_excNone;
}

static void mnemonics__xval(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    if (!machine_specific_instructions) {
        return mnemonics__reserved(addr, instr, mnemonics, comments);
    }

    string_printf(mnemonics, "_xval");
}
