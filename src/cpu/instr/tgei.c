if (CPU_64BIT_MODE(cpu))
			cond = (((int64_t) urrs.val) >=
			    ((int64_t) sign_extend_16_64(ii.imm)));
		else
			cond = (((int32_t) urrs.lo) >=
			    ((int32_t) sign_extend_16_32(ii.imm)));
		TRAP(cond, res);