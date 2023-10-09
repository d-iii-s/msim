static r4k_exc_t instr_addi(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    uint32_t rs = cpu->regs[instr.i.rs].lo;
    uint32_t imm = sign_extend_16_32(instr.i.imm);
    uint32_t sum = rs + imm;

    if (!((rs ^ imm) & SBIT32) && ((rs ^ sum) & SBIT32))
        return r4k_excOv;

    cpu->regs[instr.i.rt].val = sign_extend_32_64(sum);
    return r4k_excNone;
}

static void mnemonics_addi(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "addi");
    disassemble_rt_rs_imm(instr, mnemonics, comments);
}
