if (CPU_64BIT_INSTRUCTION(cpu)) {
			if (CP0_USABLE(cpu))
				cpu->regs[ii.rt].val = cpu->cp0[ii.rd].val;
			else
				CP0_TRAP_UNUSABLE(cpu, res);
		} else
			res = excRI;