static r4k_exc_t instr_divu(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	uint32_t rt = cpu->regs[instr.r.rt].lo;
	
	if (rt == 0) {
		cpu->loreg.val = 0;
		cpu->hireg.val = 0;
	} else {
		uint32_t rs = cpu->regs[instr.r.rs].lo;
		
		cpu->loreg.val = sign_extend_32_64(rs / rt);
		cpu->hireg.val = sign_extend_32_64(rs % rt);
	}
	
	return r4k_excNone;
}

static void mnemonics_divu(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "divu");
	disassemble_rs_rt(instr, mnemonics, comments);
}
