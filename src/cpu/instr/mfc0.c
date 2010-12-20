if (CP0_USABLE(cpu)) {
			cpu->regs[ii.rt].val = sign_extend_32_64(cpu->cp0[ii.rd].lo);
		} else
			CP0_TRAP_UNUSABLE(cpu, res);