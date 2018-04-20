static exc_t instr__xtr0(r4k_cpu_t *cpu, instr_t instr)
{
	alert("XTR0: Trace mode off");
	machine_trace = false;
	return excNone;
}
