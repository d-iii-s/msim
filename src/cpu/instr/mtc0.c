static exc_t instr_mtc0(cpu_t *cpu, instr_t instr)
{
	if (CP0_USABLE(cpu)) {
		reg64_t reg = cpu->regs[instr.r.rt];
		
		switch (instr.r.rd) {
		/* 0 */
		case cp0_Index:
			cp0_index(cpu).val = reg.val & UINT32_C(0x003f);
			break;
		case cp0_Random:
			/* Ignored, read-only */
			break;
		case cp0_EntryLo0:
			cp0_entrylo0(cpu).val = reg.val & UINT32_C(0x3fffffff);
			break;
		case cp0_EntryLo1:
			cp0_entrylo1(cpu).val = reg.val & UINT32_C(0x3fffffff);
			break;
		case cp0_Context:
			cp0_context(cpu).val = reg.val & UINT32_C(0xfffffff0);
			break;
		case cp0_PageMask:
			cp0_pagemask(cpu).val = 0;
			if (((reg.val & cp0_pagemask_mask_mask) == UINT32_C(0x0))
			    || ((reg.val & cp0_pagemask_mask_mask) == UINT32_C(0x6000))
			    || ((reg.val & cp0_pagemask_mask_mask) == UINT32_C(0x1e000))
			    || ((reg.val & cp0_pagemask_mask_mask) == UINT32_C(0x7e000))
			    || ((reg.val & cp0_pagemask_mask_mask) == UINT32_C(0x1fe000))
			    || ((reg.val & cp0_pagemask_mask_mask) == UINT32_C(0x7fe000))
			    || ((reg.val & cp0_pagemask_mask_mask) == UINT32_C(0x1ffe000)))
				cp0_pagemask(cpu).val = reg.val & cp0_pagemask_mask_mask;
			else
				alert("R4000: Invalid value for PageMask (MTC0)");
			break;
		case cp0_Wired:
			cp0_random(cpu).val = 47;
			cp0_wired(cpu).val = reg.val & UINT32_C(0x003f);
			if (cp0_wired(cpu).val > 47)
				alert("R4000: Invalid value for Wired (MTC0)");
			break;
		case cp0_Res1:
			/* Ignored, reserved */
			break;
		
		/* 8 */
		case cp0_BadVAddr:
			/* Ignored, read-only */
			break;
		case cp0_Count:
			cp0_count(cpu).val = reg.lo;
			break;
		case cp0_EntryHi:
			cp0_entryhi(cpu).val = reg.val & UINT32_C(0xfffff0ff);
			break;
		case cp0_Compare:
			cp0_compare(cpu).val = reg.lo;
			cp0_cause(cpu).val &= ~(1 << cp0_cause_ip7_shift);
			break;
		case cp0_Status:
			cp0_status(cpu).val = reg.val & UINT32_C(0xff77ff1f);
			break;
		case cp0_Cause:
			cp0_cause(cpu).val &= ~(cp0_cause_ip0_mask | cp0_cause_ip1_mask);
			cp0_cause(cpu).val |= reg.val & (cp0_cause_ip0_mask | cp0_cause_ip1_mask);
			break;
		case cp0_EPC:
			cp0_epc(cpu).val = reg.val;
			break;
		case cp0_PRId:
			/* Ignored, read-only */
			break;
		
		/* 16 */
		case cp0_Config:
			/* Ignored for simulation */
			cp0_config(cpu).val = reg.val & UINT32_C(0xffffefff);
			break;
		case cp0_LLAddr:
			cp0_lladdr(cpu).val = reg.val;
			break;
		case cp0_WatchLo:
			cp0_watchlo(cpu).val = reg.val & (~cp0_watchlo_res_mask);
			cpu->waddr = cp0_watchhi_paddr1(cpu);
			cpu->waddr <<= (32 - cp0_watchlo_paddr0_shift);
			cpu->waddr |= cp0_watchlo_paddr0(cpu);
			break;
		case cp0_WatchHi:
			cp0_watchhi(cpu).val = reg.val & (~cp0_watchhi_res_mask);
			cpu->waddr = cp0_watchhi_paddr1(cpu);
			cpu->waddr <<= (32 - cp0_watchlo_paddr0_shift);
			cpu->waddr |= cp0_watchlo_paddr0(cpu);
			break;
		case cp0_XContext:
			// FIXME TODO
			ASSERT(false);
			break;
		case cp0_Res2:
			/* Ignored, reserved */
			break;
		case cp0_Res3:
			/* Ignored, reserved */
			break;
		case cp0_Res4:
			/* Ignored, reserved */
			break;
		
		/* 24 */
		case cp0_Res5:
			/* Ignored, reserved */
			break;
		case cp0_Res6:
			/* Ignored, reserved */
			break;
		case cp0_ECC:
			/* Ignored for simulation */
			cp0_ecc(cpu).val = (reg.val & cp0_ecc_ecc_mask) << cp0_ecc_ecc_shift;
			break;
		case cp0_CacheErr:
			/* Ignored, read-only */
			break;
		case cp0_TagLo:
			cp0_taglo(cpu).val = reg.val;
			break;
		case cp0_TagHi:
			cp0_taghi(cpu).val = reg.val;
			break;
		case cp0_ErrorEPC:
			cp0_errorepc(cpu).val = reg.val;
			break;
		case cp0_Res7:
			/* Ignored */
			break;
		default:
			alert("R4000: Undefined CP0 register to set");
		}
		
		return excNone;
	}
	
	/* Coprocessor unusable */
	cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
	return excCpU;
}

static void mnemonics_mtc0(ptr64_t addr, instr_t instr,
    string_t *mnemonics, string_t *comments)
{
	string_printf(mnemonics, "mtc0");
	disassemble_rt_cp0(instr, mnemonics, comments);
}
