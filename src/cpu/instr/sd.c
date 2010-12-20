if (CPU_64BIT_INSTRUCTION(cpu)) {
			addr.ptr = urrs.val + sign_extend_16_64(ii.imm);
			res = cpu_write_mem64(cpu, addr, urrt.val, true);
		} else
			res = excRI;