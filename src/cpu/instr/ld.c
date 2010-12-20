if (CPU_64BIT_INSTRUCTION(cpu)) {
			addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
			res = cpu_read_mem64(cpu, addr, &utmp64, true);
			if (res == excNone)
				cpu->regs[ii.rt].val = utmp64;
		} else
			res = excRI;