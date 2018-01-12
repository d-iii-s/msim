static exc_t instr__xrd(cpu_t *cpu, instr_t instr)
{
	alert("XRD: Register dump");
	reg_dump(cpu);
	return excNone;
}
