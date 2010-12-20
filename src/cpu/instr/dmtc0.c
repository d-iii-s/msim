if (!CPU_64BIT_INSTRUCTION(cpu)) {
			res = excRI;
			break;
		}
		/* No break -> MTC0 */