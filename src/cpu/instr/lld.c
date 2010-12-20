if (CPU_64BIT_INSTRUCTION(cpu)) {
			/* Compute virtual target address
			   and issue read operation */
			addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
			res = cpu_read_mem64(cpu, addr, &utmp64, true);
			
			if (res == excNone) {  /* If the read operation has been successful */
				/* Store the value */
				cpu->regs[ii.rt].val = utmp64;
				
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
		} else
			res = excRI;