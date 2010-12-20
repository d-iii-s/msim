utmp64 = urrs.val + sign_extend_16_64(ii.imm);
		addr.ptr = utmp64 & ((uint64_t) ~UINT64_C(0x03));
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {
			unsigned int index = utmp64 & 0x03U;
			utmp32b = cpu->regs[ii.rt].lo & shift_tab_left[index].mask;
			utmp32b |= utmp32 << shift_tab_left[index].shift;
			cpu->regs[ii.rt].val = sign_extend_32_64(utmp32b);
		}