if (CPU_64BIT_INSTRUCTION(cpu)) {
			utmp64 = urrs.val + urrt.val;
			
			if (!((urrs.val ^ urrt.val) & SBIT64) &&
			    ((urrs.val ^ utmp64) & SBIT64)) {
				res = excOv;
				break;
			}
			
			cpu->regs[ii.rd].val = utmp64;
		} else
			res = excRI;