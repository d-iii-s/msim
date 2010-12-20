if (CP0_USABLE(cpu)) {
			uint32_t i = cp0_index_index(cpu);
			
			if (i > 47) {
				alert("R4000: Invalid value in Index (TLBR)");
				cp0_pagemask(cpu).val = 0;
				cp0_entryhi(cpu).val = 0;
				cp0_entrylo0(cpu).val = 0;
				cp0_entrylo1(cpu).val = 0;
			} else {
				cp0_pagemask(cpu).val = (~cpu->tlb[i].mask) & UINT32_C(0x01ffe000);
				cp0_entryhi(cpu).val = cpu->tlb[i].vpn2 | cpu->tlb[i].asid;
				
				cp0_entrylo0(cpu).val = (cpu->tlb[i].pg[0].pfn >> 6)
				    | (cpu->tlb[i].pg[0].cohh << 3)
				    | ((cpu->tlb[i].pg[0].dirty ? 1 : 0) << 2)
				    | ((cpu->tlb[i].pg[0].valid ? 1 : 0) << 1)
				    | (cpu->tlb[i].global ? 1 : 0);
				
				cp0_entrylo1(cpu).val = (cpu->tlb[i].pg[1].pfn >> 6)
				    | (cpu->tlb[i].pg[1].cohh << 3)
				    | ((cpu->tlb[i].pg[1].dirty ? 1 : 0) << 2)
				    | ((cpu->tlb[i].pg[1].valid ? 1 : 0) << 1)
				    | (cpu->tlb[i].global ? 1 : 0);
			}
		} else
			CP0_TRAP_UNUSABLE(cpu, res);