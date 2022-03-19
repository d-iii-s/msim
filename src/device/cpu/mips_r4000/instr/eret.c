static exc_t instr_eret(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (CP0_USABLE(cpu)) {
		/* ERET breaks LL-SC (LLD-SCD) address tracking */
		cpu->llbit = false;
		sc_unregister(cpu->procno);
		
		/* Delay slot test */
		if (cpu->branch != BRANCH_NONE)
			alert("R4000: ERET in a branch delay slot");
		
		if (cp0_status_erl(cpu)) {
			/* Error level */
			cpu->pc_next.ptr = cp0_errorepc(cpu).val;
			cp0_status(cpu).val &= ~cp0_status_erl_mask;
		} else {
			/* Exception level */
			cpu->pc_next.ptr = cp0_epc(cpu).val;
			cp0_status(cpu).val &= ~cp0_status_exl_mask;
		}
		
		return excNone;
	}
	
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}

static void mnemonics_eret(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "eret");
}
