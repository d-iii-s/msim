addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
		res = cpu_write_mem16(cpu, addr, (uint16_t) urrt.val, true);