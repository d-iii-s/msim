static exc_t instr__warning(cpu_t *cpu, instr_t instr)
{
	if (!machine_undefined) {
		alert("Undefined instruction (silent exception)");
		machine_undefined = true;
	}
	
	return excNone;
}
