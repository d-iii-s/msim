static exc_t instr__warning(cpu_t *cpu, instr_t instr)
{
	alert("Undefined instruction (silent exception)");
	return excNone;
}
