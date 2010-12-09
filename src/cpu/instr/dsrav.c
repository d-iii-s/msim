if (CPU_64BIT_INSTRUCTION(cpu))
			cpu->regs[ii.rd].val =
			    (uint64_t) (((int64_t) urrt.val) >> (urrs.val & UINT64_C(0x003f)));
		else
			res = excRI;