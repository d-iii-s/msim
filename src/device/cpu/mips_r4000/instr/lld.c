static exc_t instr_lld(r4k_cpu_t *cpu, instr_t instr)
{
	if (CPU_64BIT_INSTRUCTION(cpu)) {
		/* Compute virtual target address
		   and issue read operation */
		ptr64_t addr;
		addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
		
		uint64_t val;
		exc_t res = cpu_read_mem64(cpu, addr, &val, true);
		
		if (res == excNone) {  /* If the read operation has been successful */
			/* Store the value */
			cpu->regs[instr.i.rt].val = val;
			
			/* Since we need physical address to track, issue the
			   address conversion. It can't fail now. */
			ptr36_t phys;
			convert_addr(cpu, addr, &phys, false, false);
			
			/* Register address for tracking. */
			sc_register(cpu);
			cpu->llbit = true;
			cpu->lladdr = phys;
		} else {
			/* Invalid address; Cancel the address tracking */
			sc_unregister(cpu);
			cpu->llbit = false;
		}
		
		return res;
	}
	
	return excRI;
}

static void mnemonics_lld(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lld");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
