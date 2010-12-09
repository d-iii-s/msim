if (CPU_64BIT_INSTRUCTION(cpu))
			cpu->regs[ii.rd].val = urrt.val << (ii.sa + 32);
		else
			res = excRI;