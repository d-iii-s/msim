static exc_t instr_mult(r4k_cpu_t *cpu, instr_t instr)
{
	uint32_t rs = cpu->regs[instr.r.rs].lo;
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	/* Quick test */
	if ((rs == 0) || (rt == 0)) {
		cpu->loreg.val = 0;
		cpu->hireg.val = 0;
		return excNone;
	}
	
	uint64_t res = ((int64_t) sign_extend_32_64(rs)) *
	    ((int64_t) sign_extend_32_64(rt));
	cpu->loreg.val = sign_extend_32_64((uint32_t) res);
	cpu->hireg.val = sign_extend_32_64((uint32_t) (res >> 32));
	
	return excNone;
}

static void mnemonics_mult(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mult");
	disassemble_rs_rt(instr, mnemonics, comments);
}
