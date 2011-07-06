static exc_t instr_lui(cpu_t *cpu, instr_t instr)
{
	cpu->regs[instr.i.rt].val =
	    sign_extend_32_64(((uint32_t) instr.i.imm) << 16);
	
	return excNone;
}

static void mnemonics_lui(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "lui");
	disassemble_rt_uimm(instr, mnemonics, comments);
}
