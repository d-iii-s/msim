utmp32 = urrs.lo + urrt.lo;
		
		if (!((urrs.lo ^ urrt.lo) & SBIT32) &&
		    ((urrs.lo ^ utmp32) & SBIT32)) {
			res = excOv;
			break;
		}
		
		cpu->regs[ii.rd].val = sign_extend_32_64(utmp32);