addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = sign_extend_32_64(utmp32);