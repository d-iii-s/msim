static exc_t instr__xrd(cpu_t *cpu, instr_t instr)
{
	if (!machine_specific_instructions) {
		return instr__reserved(cpu, instr);
	}

	alert("XRD: Register dump");
	reg_dump(cpu);
	return excNone;
}
