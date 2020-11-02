static exc_t instr__xint(cpu_t *cpu, instr_t instr)
{
	if (input_is_terminal() || machine_allow_interactive_without_tty) {
		alert("XINT: Interactive mode");
		machine_interactive = true;
	} else {
		alert("XINT: Machine halt when no tty available.");
		machine_halt = true;
	}
	return excNone;
}

static void mnemonics__xint(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "_xint");
}
