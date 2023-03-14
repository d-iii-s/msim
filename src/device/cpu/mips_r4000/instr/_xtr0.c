static r4k_exc_t instr__xtr0(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	if (!machine_specific_instructions) {
		return instr__reserved(cpu, instr);
	}

	alert("XTR0: Trace mode off");

	machine_trace = false;
	return r4k_excNone;
}

static void mnemonics__xtr0(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	if (!machine_specific_instructions) {
		return mnemonics__reserved(addr, instr, mnemonics, comments);
	}

	string_printf(mnemonics, "_xtr0");
}
