utmp32 = sign_extend_16_32(ii.imm);
		utmp32b = urrs.lo + utmp32;
		
		if (!((urrs.lo ^ utmp32) & SBIT32) &&
		    ((urrs.lo ^ utmp32b) & SBIT32)) {
			res = excOv;
			break;
		}
		
		cpu->regs[ii.rt].val = sign_extend_32_64(utmp32b);