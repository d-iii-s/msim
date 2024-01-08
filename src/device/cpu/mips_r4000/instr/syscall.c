static r4k_exc_t instr_syscall(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    return r4k_excSys;
}

static void mnemonics_syscall(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    if (instr.sys.code == 0) {
        string_printf(mnemonics, "syscall");
    } else {
        string_printf(mnemonics, "syscall %#x", instr.sys.code);
    }
}
