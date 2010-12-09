if (CPU_64BIT_INSTRUCTION(cpu))
			cpu->regs[ii.rd].val =
			    (uint64_t) (((int64_t) urrt.val) >> ii.sa);
		else
			res = excRI;