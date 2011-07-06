static exc_t instr_syscall(cpu_t *cpu, instr_t instr)
{
	return excSys;
}

static void mnemonics_syscall(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	if (instr.sys.code == 0)
		string_printf(mnemonics, "syscall");
	else
		string_printf(mnemonics, "syscall %#x", instr.sys.code);
}
