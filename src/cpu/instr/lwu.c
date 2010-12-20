if (CPU_64BIT_INSTRUCTION(cpu)) {
			addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
			res = cpu_read_mem32(cpu, addr, &utmp32, true);
			if (res == excNone)
				cpu->regs[ii.rt].val = utmp32;
		} else
			res = excRI;