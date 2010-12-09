if (CPU_64BIT_INSTRUCTION(cpu))
			cpu->regs[ii.rd].val = urrs.val - urrt.val;
		else
			res = excRI;
		break;