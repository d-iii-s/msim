if (cp0_status_cu2(cpu)) {
			/* Ignored */
		} else {
			/* Coprocessor unusable */
			res = excCpU;
			cp0_cause(cpu).val &= ~cp0_cause_ce_mask;
			cp0_cause(cpu).val |= cp0_cause_ce_cu2;
		}
		break;