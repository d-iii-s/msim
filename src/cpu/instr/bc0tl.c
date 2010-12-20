static exc_t instr_bc0tl(cpu_t *cpu, instr_t instr)
{
	if (CP0_USABLE(cpu)) {
		/* Ignore (always true) */
		cpu->pc_next.ptr +=
		    (((int64_t) sign_extend_16_64(instr.i.imm)) << TARGET_SHIFT);
		cpu->branch = BRANCH_COND;
		return excJump;
	}
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}
