static exc_t instr__xval(cpu_t *cpu, instr_t instr)
{
	alert("XVAL: Register a0 = %#" PRIx64 " = %" PRIu64 " (%" PRId64 ")",
	    cpu->regs[4].val, cpu->regs[4].val, cpu->regs[4].val);
	return excNone;
}
