if (CPU_64BIT_MODE(cpu))
			cond = (urrs.val < ((uint64_t) ii.imm));
		else
			cond = (urrs.lo < ((uint32_t) ii.imm));
		TRAP(cond, res);