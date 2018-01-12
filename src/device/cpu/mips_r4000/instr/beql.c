static exc_t instr_beql(cpu_t *cpu, instr_t instr)
{
	bool cond;
	
	if (CPU_64BIT_MODE(cpu))
		cond = (cpu->regs[instr.i.rs].val == cpu->regs[instr.i.rt].val);
	else
		cond = (cpu->regs[instr.i.rs].lo == cpu->regs[instr.i.rt].lo);
	
	if (cond) {
		cpu->pc_next.ptr +=
		    (((int64_t) sign_extend_16_64(instr.i.imm)) << TARGET_SHIFT);
		cpu->branch = BRANCH_COND;
		return excJump;
	}
	
	cpu->pc_next.ptr += 4;
	return excNone;
}

static void mnemonics_beql(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "beql");
	disassemble_rs_rt_offset(addr, instr, mnemonics, comments);
}
