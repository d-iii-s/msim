if (CPU_64BIT_INSTRUCTION(cpu)) {
			if (!cpu->llbit) {
				/* If we are not tracking LLD-SCD,
				   then SCD has to fail */
				cpu->regs[ii.rt].val = 0;
			} else {
				/* We do track LLD-SCD address */
				
				/* Compute target address */
				addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
				
				/* Perform the write operation */
				res = cpu_write_mem64(cpu, addr, urrt.val, true);
				if (res == excNone) {
					/* The operation has been successful,
					   write the result, but... */
					cpu->regs[ii.rt].val = 1;
					
					/* ...we are too polite if LLD and SCD addresses differ.
					   In such a case, the behaviour of SCD is undefined.
					   Let's check that. */
					ptr36_t phys;
					convert_addr(cpu, addr, &phys, false, false);
					
					/* sc_addr now contains physical target address */
					if (phys != cpu->lladdr) {
						/* LLD and SCD addresses do not match ;( */
						alert("R4000: LLD/SCD addresses do not match");
					}
				}
				
				/* SCD always stops LLD-SCD address tracking */
				unregister_sc(cpu);
				cpu->llbit = false;
			}
		} else
			res = excRI;