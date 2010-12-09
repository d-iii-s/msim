if (CPU_64BIT_INSTRUCTION(cpu))
			multiply_s64(cpu, urrs.val, urrt.val);
		else
			res = excRI;