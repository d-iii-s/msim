static exc_t instr_swr(cpu_t *cpu, instr_t instr)
{
	ptr64_t base;
	base.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	ptr64_t addr;
	addr.ptr = base.ptr & ((uint64_t) ~UINT64_C(0x03));
	
	uint32_t val;
	exc_t res = cpu_read_mem32(cpu, addr, &val, true);
	
	if (res == excNone) {
		unsigned int index = base.ptr & 0x03U;
		val &= shift_tab_right_store[index].mask;
		val |= (cpu->regs[instr.i.rt].lo << shift_tab_right_store[index].shift);
		
		res = cpu_write_mem32(cpu, addr, val, true);
	}
	
	return res;
}
