addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
		res = cpu_read_mem16(cpu, addr, &utmp16, true);
		if (res == excNone)
			cpu->regs[ii.rt].val = utmp16;