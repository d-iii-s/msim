static exc_t instr__xtr0(cpu_t *cpu, instr_t instr)
{
	if (!machine_specific_instructions) {
		return instr__reserved(cpu, instr);
	}

	alert("XTR0: Trace mode off");

	machine_trace = false;
	return excNone;
}
