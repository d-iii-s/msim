if (CPU_64BIT_MODE(cpu))
			cond = (((int64_t) urrs.val) < ((int64_t) urrt.val));
		else
			cond = (((int32_t) urrs.lo) < ((int32_t) urrt.lo));
		TRAP(cond, res);