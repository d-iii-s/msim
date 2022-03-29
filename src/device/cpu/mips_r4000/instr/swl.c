static r4k_exc_t instr_swl(r4k_cpu_t *cpu, r4k_instr_t instr)
{
	ptr64_t base;
	base.ptr = cpu->regs[instr.i.rs].val + sign_extend_16_64(instr.i.imm);
	
	ptr64_t addr;
	addr.ptr = base.ptr & ((uint64_t) ~UINT64_C(0x03));
	
	uint32_t val;
	r4k_exc_t res = r4k_read_mem32(cpu, addr, &val, true);
	
	if (res == r4k_excNone) {
		unsigned int index = base.ptr & 0x03U;
		val &= shift_tab_left_store[index].mask;
		val |= (cpu->regs[instr.i.rt].lo >> shift_tab_left_store[index].shift)
		    & (~shift_tab_left_store[index].mask);
		
		res = cpu_write_mem32(cpu, addr, val, true);
	}
	
	return res;
}

static void mnemonics_swl(ptr64_t addr, r4k_instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "swl");
	disassemble_rt_offset_base(instr, mnemonics, comments);
}
