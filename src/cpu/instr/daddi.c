if (CPU_64BIT_INSTRUCTION(cpu)) {
			utmp64 = sign_extend_16_64(ii.imm);
			utmp64b = urrs.val + utmp64;
			
			if (!((urrs.val ^ utmp64) & SBIT64) &&
			    ((urrs.val ^ utmp64b) & SBIT64)) {
				res = excOv;
				break;
			}
			
			cpu->regs[ii.rt].val = utmp64b;
		} else
			res = excRI;