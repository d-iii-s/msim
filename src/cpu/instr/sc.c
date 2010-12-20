if (!cpu->llbit) {
			/* If we are not tracking LL-SC,
			   then SC has to fail */
			cpu->regs[ii.rt].val = 0;
		} else {
			/* We do track LL-SC address */
			
			/* Compute target address */
			addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
			
			/* Perform the write operation */
			res = cpu_write_mem32(cpu, addr, urrt.lo, true);
			if (res == excNone) {
				/* The operation has been successful,
				   write the result, but... */
				cpu->regs[ii.rt].val = 1;
				
				/* ...we are too polite if LL and SC addresses differ.
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
			unregister_sc(cpu);
			cpu->llbit = false;
		}