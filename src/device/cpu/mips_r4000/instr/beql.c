static r4k_exc_t instr_beql(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    bool cond;

    if (CPU_64BIT_MODE(cpu))
        cond = (cpu->regs[instr.i.rs].val == cpu->regs[instr.i.rt].val);
    else
        cond = (cpu->regs[instr.i.rs].lo == cpu->regs[instr.i.rt].lo);

    if (cond) {
        cpu->pc_next.ptr +=
            (((int64_t) sign_extend_16_64(instr.i.imm)) << TARGET_SHIFT);
        cpu->branch = BRANCH_COND;
        return r4k_excJump;
    }

    cpu->pc_next.ptr += 4;
    return r4k_excNone;
}

static void mnemonics_beql(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "beql");
    disassemble_rs_rt_offset(addr, instr, mnemonics, comments);
}
