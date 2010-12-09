if (CPU_64BIT_INSTRUCTION(cpu))
			cpu->regs[ii.rt].val = urrs.val + sign_extend_16_64(ii.imm);
		else
			res = excRI;