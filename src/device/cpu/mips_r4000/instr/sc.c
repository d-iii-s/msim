static exc_t instr_sc(cpu_t *cpu, instr_t instr)
{
	if (!cpu->llbit) {
		/* If we are not tracking LL-SC,
		   then SC has to fail */
		cpu->regs[instr.i.rt].val = 0;
		return excNone;
	}
	
	/* We do track LL-SC address */
	
	/* Compute target address */
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	/* Perform the write operation */
	exc_t res = cpu_write_mem32(cpu, addr, cpu->regs[instr.i.rt].lo, true);
	if (res == excNone) {
		/* The operation has been successful,
		   write the result, but ... */
		cpu->regs[instr.i.rt].val = 1;
		
		/* ... we are too polite if LL and SC addresses differ.
		   In such a case, the behaviour of SC is undefined.
		   Let's check that. */
		ptr36_t phys;
		convert_addr(cpu, addr, &phys, false, false);
		
		/* sc_addr now contains physical target address */
		if (phys != cpu->lladdr) {
			/* LL and SC addresses do not match ;( */
			alert("R4000: LL/SC addresses do not match");
		}
	}
	
	/* SC always stops LL-SC address tracking */
	sc_unregister(cpu);
	cpu->llbit = false;
	
	return res;
}

static void mnemonics_sc(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "sc");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
