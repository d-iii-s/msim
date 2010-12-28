static exc_t instr__xhtl(cpu_t *cpu, instr_t instr)
{
	alert("XHLT: Machine halt");
	machine_halt = true;
	return excNone;
}
