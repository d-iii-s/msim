if (CP0_USABLE(cpu)) {
			/* ERET breaks LL-SC (LLD-SCD) address tracking */
			cpu->llbit = false;
			unregister_sc(cpu);
			
			/* Delay slot test */
			if (cpu->branch != BRANCH_NONE)
				alert("R4000: ERET in a branch delay slot");
			
			if (cp0_status_erl(cpu)) {
				/* Error level */
				cpu->pc_next.ptr = cp0_errorepc(cpu).val;
				pca.ptr = cpu->pc_next.ptr + 4;
				cp0_status(cpu).val &= ~cp0_status_erl_mask;
			} else {
				/* Exception level */
				cpu->pc_next.ptr = cp0_epc(cpu).val;
				pca.ptr = cpu->pc_next.ptr + 4;
				cp0_status(cpu).val &= ~cp0_status_exl_mask;
			}
		} else
			CP0_TRAP_UNUSABLE(cpu, res);