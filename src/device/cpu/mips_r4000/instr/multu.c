static r4k_exc_t instr_multu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	uint32_t rs = cpu->regs[instr.r.rs].lo;
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	/* Quick test */
	if ((rs == 0) || (rt == 0)) {
		cpu->loreg.val = 0;
		cpu->hireg.val = 0;
		return r4k_excNone;
	}
	
	uint64_t res = ((uint64_t) rs) * ((uint64_t) rt);
	cpu->loreg.val = sign_extend_32_64((uint32_t) res);
	cpu->hireg.val = sign_extend_32_64((uint32_t) (res >> 32));
	
	return r4k_excNone;
}

static void mnemonics_multu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "multu");
	disassemble_rs_rt(instr, mnemonics, comments);
}
