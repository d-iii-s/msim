cpu->regs[ii.rt].val =
		    sign_extend_32_64(urrs.lo + sign_extend_16_32(ii.imm));