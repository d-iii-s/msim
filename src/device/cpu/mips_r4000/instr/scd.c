static r4k_exc_t instr_scd(r4k_cpu_t *cpu, r4k_instr_t instr)
{
    if (CPU_64BIT_INSTRUCTION(cpu)) {
        if (!cpu->llbit) {
            /* If we are not tracking LLD-SCD,
               then SC has to fail */
            cpu->regs[instr.i.rt].val = 0;
            return r4k_excNone;
        }

        /* We do track LLD-SCD address */

        /* Compute target address */
        ptr64_t addr;
        addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);

        /* Perform the write operation */
        r4k_exc_t res = cpu_write_mem64(cpu, addr, cpu->regs[instr.i.rt].val, true);
        if (res == r4k_excNone) {
            /* The operation has been successful,
               write the result, but ... */
            cpu->regs[instr.i.rt].val = 1;

            /* ... we are too polite if LLD and SCD addresses differ.
               In such a case, the behaviour of SCD is undefined.
               Let's check that. */
            ptr36_t phys;
            r4k_convert_addr(cpu, addr, &phys, false, false);

            /* sc_addr now contains physical target address */
            if (phys != cpu->lladdr) {
                /* LLD and SCD addresses do not match ;( */
                alert("R4000: LLD/SCD addresses do not match");
            }
        }

        /* SCD always stops LLD-SCD address tracking */
        sc_unregister(cpu->procno);
        cpu->llbit = false;

        return res;
    }

    return r4k_excRI;
}

static void mnemonics_scd(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
    string_printf(mnemonics, "scd");
    disassemble_rt_offset_base(instr, mnemonics, comments);
}
