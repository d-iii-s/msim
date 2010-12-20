utmp64 = urrs.val + sign_extend_16_64(ii.imm);
		addr.ptr = utmp64 & ((uint64_t) ~UINT64_C(0x03));
		res = cpu_read_mem32(cpu, addr, &utmp32, true);
		
		if (res == excNone) {
			unsigned int index = utmp64 & 0x03U;
			utmp32 &= shift_tab_left_store[index].mask;
			utmp32 |= (urrt.lo >> shift_tab_left_store[index].shift)
			    & (~shift_tab_left_store[index].mask);
			res = cpu_write_mem32(cpu, addr, utmp32, true);
		}