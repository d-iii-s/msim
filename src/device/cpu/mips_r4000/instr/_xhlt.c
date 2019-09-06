static exc_t instr__xhlt(cpu_t *cpu, instr_t instr)
{
	if (!machine_specific_instructions) {
		return instr__reserved(cpu, instr);
	}
	alert("XHLT: Machine halt");
	machine_halt = true;
	return excNone;
}
