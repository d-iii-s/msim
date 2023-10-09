static r4k_exc_t instr__xtrc(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (!machine_specific_instructions) {
		return instr__reserved(cpu, instr);
	}

	alert("XTRC: Trace mode");

	if (!machine_trace)
		r4k_reg_dump(cpu);

	/* Update debugging information */
	memcpy(cpu->old_regs, cpu->regs, sizeof(cpu->regs));
	memcpy(cpu->old_cp0, cpu->cp0, sizeof(cpu->cp0));

	cpu->old_loreg = cpu->loreg;
	cpu->old_hireg = cpu->hireg;

	machine_trace = true;

	return r4k_excNone;
}

static void mnemonics__xtrc(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	if (!machine_specific_instructions) {
		return mnemonics__reserved(addr, instr, mnemonics, comments);
	}

	string_printf(mnemonics, "_xtrc");
}
