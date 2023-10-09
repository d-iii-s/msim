static r4k_exc_t instr_ddiv(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (CPU_64BIT_INSTRUCTION(cpu)) {
        uint64_t rt = cpu->regs[instr.r.rt].val;

        if (rt == 0) {
            cpu->loreg.val = 0;
            cpu->hireg.val = 0;
        } else {
            uint64_t rs = cpu->regs[instr.r.rs].val;

            cpu->loreg.val = (uint64_t) (((int64_t) rs) / ((int64_t) rt));
            cpu->hireg.val = (uint64_t) (((int64_t) rs) % ((int64_t) rt));
        }

        return r4k_excNone;
    }

    return r4k_excRI;
}

static void mnemonics_ddiv(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "ddiv");
    disassemble_rs_rt(instr, mnemonics, comments);
}
