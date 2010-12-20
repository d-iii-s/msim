if (CPU_64BIT_MODE(cpu))
			cond = (urrs.val == urrt.val);
		else
			cond = (urrs.lo == urrt.lo);
		TRAP(cond, res);