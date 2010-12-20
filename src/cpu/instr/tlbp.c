if (CP0_USABLE(cpu)) {
			cp0_index(cpu).val = 1 << cp0_index_p_shift;
			uint32_t xvpn2 = cp0_entryhi(cpu).val & cp0_entryhi_vpn2_mask;
			uint32_t xasid = cp0_entryhi(cpu).val & cp0_entryhi_asid_mask;
			unsigned int i;
			
			for (i = 0; i < TLB_ENTRIES; i++) {
				if ((cpu->tlb[i].vpn2 == xvpn2) &&
				    ((cpu->tlb[i].global) || (cpu->tlb[i].asid == xasid))) {
					cp0_index(cpu).val = i;
					break;
				}
			}
		} else
			CP0_TRAP_UNUSABLE(cpu, res)