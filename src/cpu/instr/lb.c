addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
		res = cpu_read_mem8(cpu, addr, &utmp8, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = sign_extend_8_64(utmp8);