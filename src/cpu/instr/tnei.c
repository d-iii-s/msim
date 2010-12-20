if (CPU_64BIT_MODE(cpu))
			cond = (urrs.val != sign_extend_16_64(ii.imm));
		else
			cond = (urrs.lo != sign_extend_16_32(ii.imm));
		TRAP(cond, res);