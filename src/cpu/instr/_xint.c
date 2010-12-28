static exc_t instr__xint(cpu_t *cpu, instr_t instr)
{
	alert("XINT: Interactive mode");
	machine_interactive = true;
	return excNone;
}
