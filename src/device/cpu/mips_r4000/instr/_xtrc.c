static exc_t instr__xtrc(cpu_t *cpu, instr_t instr)
{
	if (!machine_specific_instructions) {
		return instr__reserved(cpu, instr);
	}

	alert("XTRC: Trace mode");

	if (!machine_trace)
		reg_dump(cpu);
	
	/* Update debugging information */
	memcpy(cpu->old_regs, cpu->regs, sizeof(cpu->regs));
	memcpy(cpu->old_cp0, cpu->cp0, sizeof(cpu->cp0));
	
	cpu->old_loreg = cpu->loreg;
	cpu->old_hireg = cpu->hireg;
	
	machine_trace = true;
	
	return excNone;
}
