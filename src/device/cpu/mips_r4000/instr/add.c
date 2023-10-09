static r4k_exc_t instr_add(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    uint32_t rs = cpu->regs[instr.r.rs].lo;
    uint32_t rt = cpu->regs[instr.r.rt].lo;
    uint32_t sum = rs + rt;

    if (!((rs ^ rt) & SBIT32) && ((rs ^ sum) & SBIT32))
        return r4k_excOv;

    cpu->regs[instr.r.rd].val = sign_extend_32_64(sum);
    return r4k_excNone;
}

static void mnemonics_add(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "add");
    disassemble_rd_rs_rt(instr, mnemonics, comments);
}
