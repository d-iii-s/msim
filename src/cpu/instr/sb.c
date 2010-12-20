addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
		res = cpu_write_mem8(cpu, addr, (uint8_t) urrt.val, true);