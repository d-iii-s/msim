static r4k_exc_t instr_jalr(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    cpu->regs[31].val = cpu->pc.ptr + 8;
    cpu->pc_next.ptr = cpu->regs[instr.r.rs].val;
    cpu->branch = BRANCH_COND;
    return r4k_excJump;
}

static void mnemonics_jalr(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "jalr");
    if (instr.r.rd == 31)
        disassemble_rs(instr, mnemonics, comments);
    else
        disassemble_rd_rs(instr, mnemonics, comments);
}
