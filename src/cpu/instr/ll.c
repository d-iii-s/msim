/* Compute virtual target address
		   and issue read operation */
		addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {  /* If the read operation has been successful */
			/* Store the value */
			cpu->regs[ii.rt].val = sign_extend_32_64(utmp32);
			
			/* Since we need physical address to track, issue the
			   address conversion. It can't fail now. */
			ptr36_t phys;
			convert_addr(cpu, addr, &phys, false, false);
			
			/* Register address for tracking. */
			register_sc(cpu);
			cpu->llbit = true;
			cpu->lladdr = phys;
		} else {
			/* Invalid address; Cancel the address tracking */
			unregister_sc(cpu);
			cpu->llbit = false;
		}