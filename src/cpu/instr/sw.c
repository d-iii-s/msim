addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
		res = cpu_write_mem32(cpu, addr, urrt.lo, true);