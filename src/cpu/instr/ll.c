static exc_t instr_ll(cpu_t *cpu, instr_t instr)
{
	/* Compute virtual target address
	   and issue read operation */
	ptr64_t addr;
	addr.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	uint32_t val;
	exc_t res = cpu_read_mem32(cpu, addr, &val, true);
	
	if (res == excNone) {  /* If the read operation has been successful */
		/* Store the value */
		cpu->regs[instr.i.rt].val = sign_extend_32_64(val);
		
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
