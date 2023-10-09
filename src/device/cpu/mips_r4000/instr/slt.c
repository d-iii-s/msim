static r4k_exc_t instr_slt(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (CPU_64BIT_MODE(cpu)) {
        uint64_t rs = cpu->regs[instr.r.rs].val;
        uint64_t rt = cpu->regs[instr.r.rt].val;

        cpu->regs[instr.r.rd].val = ((int64_t) rs) < ((int64_t) rt);
    } else {
        uint32_t rs = cpu->regs[instr.r.rs].lo;
        uint32_t rt = cpu->regs[instr.r.rt].lo;

        cpu->regs[instr.r.rd].val = ((int32_t) rs) < ((int32_t) rt);
    }

    return r4k_excNone;
}

static void mnemonics_slt(ptr64_t addr, r4k_instr_t instr,
        string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "slt");
    disassemble_rd_rs_rt(instr, mnemonics, comments);
}
